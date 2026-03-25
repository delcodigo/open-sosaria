#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "playerOverworld.h"
#include "worldMap.h"
#include "engine/geometry.h"
#include "engine/camera.h"
#include "engine/input.h"
#include "data/saveAndLoad.h"
#include "data/bevery.h"
#include "data/enemy.h"
#include "ui/uiConsole.h"
#include "ui/uiztats.h"
#include "scenes/sceneDiskLoader.h"
#include "scenes/sceneOverworld.h"
#include "scenes/sceneTown.h"
#include "maths/matrix4.h"
#include "vehicleOverworld.h"
#include "config.h"
#include "utils.h"
#include "memory.h"

static PLAYER_STATE playerState = PLAYER_STATE_IDLE;
static Geometry playerOverworldGeometry;
static float transformationMatrix[16];

static Geometry *enemyGeometry = NULL;
static float enemyTransformationMatrix[16];
static bool renderEnemy = false;

static READY_STEP readyStep = READY_STEP_START;
static Textfield inputTextfieldBuffer;
static char selectedWeapon[3] = {0};
static bool areKeysReleased = true;

static float keyRepeatDelay = 0;
static float waitingTime = 0.0f;
static int lastSignpost = -1;

void playerOverworld_init() {
  geometry_setSprite(&playerOverworldGeometry, OS_TILE_WIDTH, OS_TILE_HEIGHT, 0, 0.5f, 0.125f, 1.0f);
  matrix4_setIdentity(transformationMatrix);
  matrix4_setIdentity(enemyTransformationMatrix);
  memset(&inputTextfieldBuffer, 0, sizeof(inputTextfieldBuffer));
  playerOverworld_setCameraFollow();
  inputTextfieldBuffer.maxLength = 2;
}

bool playerOverworld_updateWait() {
  if (input.space) {
    waitingTime = 0.0f;
    char waitCommand[30] = {0};
    snprintf(waitCommand, sizeof(waitCommand), "%.14s%.15s", ultimaStrings[98], ultimaStrings[99]);
    uiConsole_replaceLastMessage(waitCommand);
    keyRepeatDelay = 0.3f;
    return true;
  }

  return false;
}

bool playerOverworld_tryAndDodgeEnemies(int mx, int my) {
  if (enemyEncounter.monsterId <= 0 || 
    (float)(player.strength + player.agility) / 400.0f > rand01() ||
    player.vehicle + 2 > rand01() * 14
  ) {
    enemyEncounter.monsterId = 0;
    renderEnemy = false;
    return true;
  }

  char encounterMessage[31] = {0};
  snprintf(encounterMessage, sizeof(encounterMessage), "^1%.15s %.8s^0", ultimaStrings[121], enemyDefinitions[enemyEncounter.monsterId].name);
  uiConsole_queueMessage(encounterMessage);

  renderEnemy = true;
  if (enemyGeometry != NULL) {
    geometry_free(enemyGeometry);
    free(enemyGeometry);
    enemyGeometry = NULL;
  }

  int tile = (worldMap_getPlayerTile() >> 4) & 0x0F;
  if (tile > 2) { tile = 1; }
  float tx1 = tile * (OS_ENEMY_SPRITE_WIDTH / (float)ultimaAssets.enemySprites.width);
  float tx2 = tx1 + (OS_ENEMY_SPRITE_WIDTH / (float)ultimaAssets.enemySprites.width);

  enemyGeometry = (Geometry*) malloc(sizeof(Geometry));
  matrix4_setPosition(enemyTransformationMatrix, (player.tx + mx) * OS_TILE_WIDTH, (player.ty + my) * OS_TILE_HEIGHT, 3.0f);
  geometry_setSprite(enemyGeometry, OS_ENEMY_SPRITE_WIDTH, OS_ENEMY_SPRITE_HEIGHT, tx1, 0, tx2, 1);

  return false;
}

bool playerOverworld_updateMovement(float deltaTime) {
  int moveX = 0;
  int moveY = 0;
  char movementCommand[30] = {0};

  if (input.up) {
    moveY = -1;
    snprintf(movementCommand, sizeof(movementCommand), "%.14s%.15s", ultimaStrings[98], ultimaStrings[117]);
  } else if (input.down) {
    moveY = 1;
    snprintf(movementCommand, sizeof(movementCommand), "%.14s%.15s", ultimaStrings[98], ultimaStrings[118]);
  } else if (input.left) {
    moveX = -1;
    snprintf(movementCommand, sizeof(movementCommand), "%.14s%.15s", ultimaStrings[98], ultimaStrings[120]);
  } else if (input.right) {
    moveX = 1;
    snprintf(movementCommand, sizeof(movementCommand), "%.14s%.15s", ultimaStrings[98], ultimaStrings[119]);
  }

  if (moveX != 0 || moveY != 0) {
    waitingTime = 0.0f;
    int tx = (int)((player.tx + moveX) % OS_BTERRA_MAP_WIDTH);
    int ty = (int)((player.ty + moveY) % OS_BTERRA_MAP_HEIGHT);
    int world = ((int)(player.ty + moveY) / OS_BTERRA_MAP_HEIGHT) * 2 + ((int)(player.tx + moveX) / OS_BTERRA_MAP_WIDTH);
    int tile = (ultimaAssets.bterraMaps[world][ty][tx] >> 4) & 0x0F;

    uiConsole_replaceLastMessage(movementCommand);

    if (!playerOverworld_tryAndDodgeEnemies(moveX, moveY)) {
      keyRepeatDelay = 0.3f;
      return true;
    }

    int vehicleTile = vehiclesMap[player.ty + moveY][player.tx + moveX];

    if (vehicleTile == 0 || player.vehicle != 0) {
      if (tile == 0 && (player.vehicle < 3 || player.vehicle > 5)) {
        uiConsole_addMessage(ultimaStrings[122]);
        keyRepeatDelay = 0.3f;
        return true;
      } else if (tile == 3) {
        uiConsole_addMessage(ultimaStrings[123]);
        keyRepeatDelay = 0.3f;
        return true;
      } else if (tile == 2 && player.vehicle == 5) {
        uiConsole_addMessage(ultimaStrings[124]);
        keyRepeatDelay = 0.3f;
        return true;
      } else if (tile > 0 && (player.vehicle == 3 || player.vehicle == 4)) {
        uiConsole_addMessageFormat("%.14s%.15s", vehicleNames[player.vehicle], ultimaStrings[125]);
        keyRepeatDelay = 0.3f;
        return true;
      }
    }

    player.tx = (player.tx + moveX + OS_BTERRA_MAP_WIDTH * 2) % (OS_BTERRA_MAP_WIDTH * 2);
    player.ty = (player.ty + moveY + OS_BTERRA_MAP_HEIGHT * 2) % (OS_BTERRA_MAP_HEIGHT * 2);
    playerOverworld_setCameraFollow();
    keyRepeatDelay = 0.1f;

    player_consumeFood();

    return true;
  } else {
    waitingTime += deltaTime;
    if (waitingTime >= 5.0f) {
      waitingTime = 0.0f;
      snprintf(movementCommand, sizeof(movementCommand), "%.14s%.15s", ultimaStrings[98], ultimaStrings[99]);
      uiConsole_replaceLastMessage(movementCommand);
      player_waitPenalty();
      return true;
    }
  }

  return false;
}

static bool playerOverworld_updateZtats() {
  if (input.z == 1) {
    input.z = 2;
    uiZtats_init();
    ztatsActive = true;
    waitingTime = 0.0f;
    memset(&input, 0, sizeof(input));
    return true;
  }

  return false;
}

static bool playerOverworld_updateSave() {
  if (input.q == 1) {
    input.q = 2;
    uiConsole_addMessage(ultimaStrings[190]);
    waitingTime = 0.0f;
    memset(&input, 0, sizeof(input));
    saveGame();
    uiConsole_addMessage(ultimaStrings[197]);
    return true;
  }

  return false;
}

static bool playerOverworld_updateInfo() {
  if (input.i == 1) {
    input.i = 2;
    waitingTime = 0.0f;
    memset(&input, 0, sizeof(input));

    uiConsole_addMessage(ultimaStrings[176]);

    int tx = (int)(player.tx % OS_BTERRA_MAP_WIDTH);
    int ty = (int)(player.ty % OS_BTERRA_MAP_HEIGHT);
    int world = ((int)player.ty / OS_BTERRA_MAP_HEIGHT) * 2 + ((int)player.tx / OS_BTERRA_MAP_WIDTH);
    int tile = (ultimaAssets.bterraMaps[world][ty][tx] >> 4) & 0x0F;
    int tileType = ultimaAssets.bterraMaps[world][ty][tx] & 0x0F;

    if (tile == 0) { uiConsole_addMessage(ultimaStrings[177]); } else
    if (tile == 1) { uiConsole_addMessage(ultimaStrings[178]); } else
    if (tile == 2) { uiConsole_addMessage(ultimaStrings[179]); } else 
    if (tile == 4) { uiConsole_addMessage(placesNames[world * 20 + tileType + 1]); } else
    if (tile == 5) { uiConsole_addMessage(placesNames[world * 20 + tileType + 3]); } else
    if (tile == 6) { 
      uiConsole_addMessageFormat("%s%s", ultimaStrings[180], placesNames[world * 20 + tileType + 13]);
    } else
    if (tile == 7) { uiConsole_addMessage(placesNames[world * 20 + tileType + 5]); }

    uiConsole_addMessage(ultimaStrings[181]);

    return true;
  }

  return false;
}

static bool playerOverworld_get() {
  if (input.g == 1) {
    input.g = 2;
    waitingTime = 0.0f;
    memset(&input, 0, sizeof(input));

    char getCommand[31] = {0};
    snprintf(getCommand, sizeof(getCommand), "%.14s%.15s", ultimaStrings[98], ultimaStrings[173]);
    uiConsole_replaceLastMessage(getCommand);
    uiConsole_addMessage(ultimaStrings[174]);

    return true;
  }

  return false;
}

static bool playerOverworld_open() {
  if (input.o == 1) {
    input.o = 2;
    waitingTime = 0.0f;
    memset(&input, 0, sizeof(input));

    char openCommand[31] = {0};
    snprintf(openCommand, sizeof(openCommand), "%.14s%.15s", ultimaStrings[98], ultimaStrings[187]);
    uiConsole_replaceLastMessage(openCommand);
    uiConsole_addMessage(ultimaStrings[188]);

    return true;
  }

  return false;
}

static bool playerOverworld_drop() {
  if (input.d == 1) {
    input.d = 2;
    waitingTime = 0.0f;
    memset(&input, 0, sizeof(input));

    char dropCommand[31] = {0};
    snprintf(dropCommand, sizeof(dropCommand), "%.14s%.15s", ultimaStrings[98], ultimaStrings[162]);
    uiConsole_replaceLastMessage(dropCommand);

    return true;
  }

  return false;
}

static void playerOverworld_activateInput() {
  inputTextfieldBuffer.active = true;
  inputTextfield = &inputTextfieldBuffer;
  inputTextfield->cursorPosition = 0;
  inputTextfield->text[0] = '\0';
  inputTextfield->isDirty = false;
  areKeysReleased = false;
}

static void playerOverworld_tryAndEquipSpell(int spellIndex, const char *charUsed) {
  if (spellIndex < 0) {
    uiConsole_replaceLastMessageFormat("%.15s%s%.10s", ultimaStrings[214], charUsed, ultimaStrings[224]);
    return;
  }

  if (spellIndex > 0 && player.spells[spellIndex - 1] < 1) {
    uiConsole_addMessageFormat("%.15s%s", ultimaStrings[225], spellNames[spellIndex]);
    return;
  }

  player.spell = spellIndex;
  uiConsole_addMessageFormat("%.15s%s", ultimaStrings[226], spellNames[spellIndex]);
}

static bool playerOverworld_readyStart() {
  if (input.r == 1) {
    input.r = 2;
    waitingTime = 0.0f;
    memset(&input, 0, sizeof(input));
    char readyCommand[31] = {0};
    snprintf(readyCommand, sizeof(readyCommand), "%.14s%.15s", ultimaStrings[98], ultimaStrings[198]);
    uiConsole_replaceLastMessage(readyCommand);
    uiConsole_addMessage(ultimaStrings[199]);

    areKeysReleased = false;
    playerState = PLAYER_STATE_READY_TYPE;
    readyStep = READY_STEP_SELECT_TYPE;
  }

  return false;
}

static bool playerOverworld_readySelectType() {
  if (input.w == 1) {
    input.w = 2;
    memset(selectedWeapon, 0, sizeof(selectedWeapon));
    uiConsole_addMessage(ultimaStrings[205]);
    playerOverworld_activateInput();
    readyStep = READY_STEP_WEAPON_INPUT;
  } else if (input.a == 1) {
    input.a = 2;
    uiConsole_addMessage(ultimaStrings[210]);
    playerOverworld_activateInput();
    readyStep = READY_STEP_ARMOR_EQUIP;
  } else if (input.s == 1) {
    input.s = 2;
    uiConsole_addMessage(ultimaStrings[214]);
    playerOverworld_activateInput();
    readyStep = READY_STEP_SPELL_EQUIP;
  }

  return false;
}

static bool playerOverworld_readyWeaponInput() {
  if (inputTextfield->isDirty && inputTextfield->text[0] != '\0') {
    inputTextfield->isDirty = false;

    if (selectedWeapon[0] == '\0') {
      selectedWeapon[0] = (char)toupper(inputTextfield->text[0]);
    } else if (selectedWeapon[1] == '\0') {
      selectedWeapon[1] = (char)toupper(inputTextfield->text[0]);
      readyStep = READY_STEP_WEAPON_EQUIP;
    }

    char weaponCommand[31] = {0};
    snprintf(weaponCommand, sizeof(weaponCommand), "%.14s%.2s", ultimaStrings[205], selectedWeapon);
    uiConsole_replaceLastMessage(weaponCommand);

    inputTextfield->cursorPosition = 0;
    inputTextfield->text[0] = '\0';
    areKeysReleased = false;
  }

  return false;
}

static bool playerOverworld_readyWeaponEquip() {
  char weaponCommand[31] = {0};
  readyStep = READY_STEP_RETURN_TO_IDLE;
  areKeysReleased = false;

  for (int i=0;i<OS_WEAPONS_COUNT;i++) {
    char weaponAbbreviation[3] = {weaponNames[i][0], weaponNames[i][1], '\0'};
    if (strcmp(weaponAbbreviation, selectedWeapon) == 0) {
      memset(selectedWeapon, 0, sizeof(selectedWeapon));

      if (i > 0 && player.weapons[i - 1] < 1) {
        snprintf(weaponCommand, sizeof(weaponCommand), "%.15s%.15s", ultimaStrings[207], weaponNames[i]);
        uiConsole_addMessage(weaponCommand);
        return true;
      }

      player.weapon = i - 1;
      snprintf(weaponCommand, sizeof(weaponCommand), "%.15s%.15s", ultimaStrings[208], weaponNames[i]);
      uiConsole_addMessage(weaponCommand);
      return true;
    }
  }

  snprintf(weaponCommand, sizeof(weaponCommand), "%.2s%.15s", selectedWeapon, ultimaStrings[206]);
  uiConsole_addMessage(weaponCommand);
  memset(selectedWeapon, 0, sizeof(selectedWeapon));
  return true;
}

static bool playerOverworld_readyArmorEquip() {
  if (inputTextfield->isDirty && inputTextfield->text[0] != '\0') {
    inputTextfield->isDirty = false;
    char armorChar[2] = {(char)toupper(inputTextfield->text[0]), '\0'};
    uiConsole_replaceLastMessageFormat("%.14s%.1s", ultimaStrings[210], armorChar);

    readyStep = READY_STEP_RETURN_TO_IDLE;
    areKeysReleased = false;

    for (int i=0;i<OS_ARMORS_COUNT;i++) {
      if (armorNames[i][0] == armorChar[0]) {
        if (player.armors[i - 1] < 1 && i > 0) {
          uiConsole_addMessageFormat("%.15s%.15s", ultimaStrings[212], armorNames[i]);
          return false;
        }

        uiConsole_addMessageFormat("%.15s%.15s", ultimaStrings[213], armorNames[i]);
        inputTextfield = NULL;
        player.armor = i - 1;
        return false;
      }
    }

    uiConsole_addMessageFormat("%.1s%.15s", armorChar, ultimaStrings[211]);
    return false;
  }

  return false;
}

static bool playerOverworld_readySpellEquip() {
  if (inputTextfield->isDirty && inputTextfield->text[0] != '\0') {
    char selectedSpellChar[2] = {(char)toupper(inputTextfield->text[0]), '\0'};
    inputTextfield->isDirty = false;

    if (selectedSpellChar[0] == 'L') {
      uiConsole_replaceLastMessageFormat("%.15sL %.10s", ultimaStrings[214], ultimaStrings[216]);
      readyStep = READY_STEP_SPELL_LADDER;
      areKeysReleased = false;
      return false;
    } else if (selectedSpellChar[0] == 'P') {
      uiConsole_replaceLastMessageFormat("%.12sP[RAY/ROJ] (R/J)", ultimaStrings[214]);
      readyStep = READY_STEP_SPELL_PRAYER_OR_PROJECTILE;
      areKeysReleased = false;
      return false;
    }

    int spellIndex = -1;
    for (int i=0;i<OS_SPELLS_COUNT;i++) {
      if (spellNames[i][0] == selectedSpellChar[0]) {
        spellIndex = i;
        break;
      }
    }

    playerOverworld_tryAndEquipSpell(spellIndex, selectedSpellChar);
    readyStep = READY_STEP_RETURN_TO_IDLE;
    areKeysReleased = false;
    return true;
  }

  return false;
}

static bool playerOverworld_readySpellLadder() {
  if (inputTextfield->isDirty && inputTextfield->text[0] != '\0') {
    char selectedSpellChar[3] = {'L', (char)toupper(inputTextfield->text[0]), '\0'};
    inputTextfield->isDirty = false;
    uiConsole_replaceLastMessageFormat("%.15s%s", ultimaStrings[214], selectedSpellChar);

    readyStep = READY_STEP_RETURN_TO_IDLE;
    areKeysReleased = false;

    int spellIndex = -1;
    if (selectedSpellChar[1] == 'U') {
      spellIndex = 6;
      playerOverworld_tryAndEquipSpell(spellIndex, "LU");
    } else if (selectedSpellChar[1] == 'D') {
      spellIndex = 5;
      playerOverworld_tryAndEquipSpell(spellIndex, "LD");
    } else {
      uiConsole_addMessage(ultimaStrings[220]);
    }

    return true;
  }

  return false;
}

static void playerOverworld_readyReturnToIdle() {
  inputTextfield = NULL;
  inputTextfieldBuffer.active = false;
  playerState = PLAYER_STATE_IDLE;
  readyStep = READY_STEP_START;
}

static bool playerOverworld_readPrayerOrProjectile() {
  if (inputTextfield->isDirty && inputTextfield->text[0] != '\0') {
    char selectedSpellChar[3] = {'P', (char)toupper(inputTextfield->text[0]), '\0'};
    inputTextfield->isDirty = false;
    uiConsole_replaceLastMessageFormat("%.15s%s", ultimaStrings[214], selectedSpellChar);

    readyStep = READY_STEP_RETURN_TO_IDLE;
    areKeysReleased = false;

    int spellIndex = -1;
    if (selectedSpellChar[1] == 'R') {
      spellIndex = 0;
      playerOverworld_tryAndEquipSpell(spellIndex, "PR");
    } else if (selectedSpellChar[1] == 'J') {
      spellIndex = 3;
      playerOverworld_tryAndEquipSpell(spellIndex, "PJ");
    } else {
      uiConsole_addMessage(ultimaStrings[220]);
    }

    return true;
  }

  return false;
}

static bool playerOverworld_ready() {
  if (!areKeysReleased) {
    if (input_areKeysReleased()) {
      areKeysReleased = true;
      if (inputTextfield != NULL){
        inputTextfield->cursorPosition = 0;
        inputTextfield->text[0] = '\0';
      }
    } else {
      return false;
    }
  }

  switch (readyStep) {
    case READY_STEP_START:
      return playerOverworld_readyStart();
    case READY_STEP_SELECT_TYPE:
      return playerOverworld_readySelectType();
    case READY_STEP_WEAPON_INPUT:
      return playerOverworld_readyWeaponInput();
    case READY_STEP_WEAPON_EQUIP:
      return playerOverworld_readyWeaponEquip();
    case READY_STEP_ARMOR_EQUIP:
      return playerOverworld_readyArmorEquip();
    case READY_STEP_SPELL_EQUIP:
      return playerOverworld_readySpellEquip();
    case READY_STEP_SPELL_LADDER:
      return playerOverworld_readySpellLadder();
    case READY_STEP_SPELL_PRAYER_OR_PROJECTILE:
      return playerOverworld_readPrayerOrProjectile();
      break;
    case READY_STEP_RETURN_TO_IDLE:
      playerOverworld_readyReturnToIdle();
      break;
  }

  return false;
}

static void playerOverworld_checkIfEnemiesDead() {
  if (enemyEncounter.hp < 0) {
    int monsterId = enemyEncounter.monsterId;

    enemyEncounter.number -= 1;
    
    player.experience += enemyDefinitions[monsterId].rank * 5;
    int goldEarned = (int)(enemyDefinitions[monsterId].rank * 4 * rand01()) + 10;
    player.gold += goldEarned;

    uiConsole_queueMessageFormat("%.15s+%d", ultimaStrings[134], goldEarned);

    if (enemyEncounter.number <= 0) {
      enemyEncounter.monsterId = -1;
      renderEnemy = false;
      if (enemyGeometry != NULL) {
        geometry_free(enemyGeometry);
        free(enemyGeometry);
        enemyGeometry = NULL;
      }
    } else {
      enemyEncounter.hp = (int)(20 * rand01() + pow(rand01(), 2) * (int)(player.time / 1000.0f)) + 5;
    }

    uiConsole_updateStats();
  }
}

static bool playerOverworld_updateAttack() {
  if (input.a == 1) {
    input.a = 2;
    waitingTime = 0.0f;
    lagTime = 1.5f;

    uiConsole_replaceLastMessageFormat("%.11s^F^1%.12s^0", ultimaStrings[98], ultimaStrings[126]);

    int monsterId = enemyEncounter.monsterId;

    if (monsterId < 6 || monsterId > 20) {
      uiConsole_queueMessage(ultimaStrings[127]);
      return true;
    }

    uiConsole_queueMessageFormat("^F^1%s^0", enemyDefinitions[monsterId].name);

    if ((monsterId < 10 || monsterId == 12) && (player.weapon < 7 || player.weapon == 11 || player.weapon == 13)) {
      uiConsole_queueMessage(ultimaStrings[128]);
      return true;
    }

    uiConsole_queueMessageFormat("%.15s%s", ultimaStrings[129], weaponNames[player.weapon]);

    if (player.weapon > 7 && player.weapon < 12) {
      uiConsole_queueMessageFormat("%.10s%.19s", weaponNames[player.weapon], ultimaStrings[135]);
      return true;
    }

    float attack = rand01() * 20.0f + (float)(player.strength + player.agility) / 5.0f + (float)player.weapon;
    int defense = enemyDefinitions[monsterId].rank + 10;
    if (attack > defense || attack > 20) {
      int damage = (int)((player.strength + player.weapon) * rand01()) + 1;
      uiConsole_queueMessageFormat("%.5s%.10s%d", ultimaStrings[131], ultimaStrings[132], damage);

      enemyEncounter.hp -= damage;

      playerOverworld_checkIfEnemiesDead();

      return true;
    }

    uiConsole_queueMessage(ultimaStrings[130]);
    
    return true;
  }

  return false;
}

static bool playerOverwolrd_cast() {
  if (input.c == 1) {
    input.c = 2;
    waitingTime = 0.0f;

    uiConsole_queueMessageFormat("%.15s%.15s", ultimaStrings[145], spellNames[player.spell]);

    if (player.spell > 0 && player.spells[player.spell] < 1) {
      uiConsole_queueMessage(ultimaStrings[146]);
      uiConsole_queueMessageFormat("%.15s%.15s", spellNames[player.spell], ultimaStrings[147]);
      return true;
    }

    player.spells[player.spell] -= 1;

    switch (player.spell) {
      case 0:
        uiConsole_queueMessageFormat("^T1%.27s", ultimaStrings[148]);
        int randomEffect = rand01() * 3 + 1;
        bool removedEnemies = false;
        if (randomEffect == 1 && enemyEncounter.monsterId > 0) {
          uiConsole_queueMessage(ultimaStrings[149]);
          enemyEncounter.monsterId = 0;
          renderEnemy = false;
          removedEnemies = true;

          if (player.gold > 20) {
            player.gold -= 20;
          }
        }

        if (!removedEnemies){
          if (randomEffect < 3 && player.health < 10) {
            player.health = 10;
            uiConsole_queueMessage(ultimaStrings[150]);
          } else if (player.food < 10) {
            player.food = 10;
            uiConsole_queueMessage(ultimaStrings[150]);
          } else {
            uiConsole_queueMessage(ultimaStrings[152]);
          }
        }
        
        uiConsole_updateStats();
        break;

      case 3:
        if (enemyEncounter.monsterId < 1) {
          uiConsole_queueMessage(ultimaStrings[153]);
        } else {
          uiConsole_queueMessageFormat("^T1%.27s", ultimaStrings[154]);

          int damage = (int)(player.wisdom / 2);
          if (player.weapon > 7 && player.weapon < 12) {
            damage += player.weapon * 2;
          }

          uiConsole_queueMessageFormat("%.10s%.10s%d", ultimaStrings[155], ultimaStrings[156], damage);

          enemyEncounter.hp -= damage;
          playerOverworld_checkIfEnemiesDead();
        }
        break;

      case 10:
        if (enemyEncounter.monsterId < 1) {
          uiConsole_queueMessage(ultimaStrings[153]);
        } else {
          uiConsole_queueMessageFormat("^T1%.27s", ultimaStrings[159]);

          enemyEncounter.monsterId = -1;
          renderEnemy = false;
        }
        break;
        
      default:
        uiConsole_queueMessage(ultimaStrings[160]);
        uiConsole_queueMessage(ultimaStrings[161]);
        break;
    }
      
    lagTime = 4.0f;
    return true;
  }
  return false;
}

void playerOverworld_updateGeometry() {
  if (playerOverworldGeometry.indexCount > 0){
    geometry_free(&playerOverworldGeometry);
  }

  float tx1 = player.vehicle * OS_TILE_WIDTH / (float) ultimaAssets.overworldTiles.width;
  float tx2 = tx1 + OS_TILE_WIDTH / (float) ultimaAssets.overworldTiles.width;
  geometry_setSprite(&playerOverworldGeometry, OS_TILE_WIDTH, OS_TILE_HEIGHT, tx1, 0.5f, tx2, 1.0f);
}

static bool playerOverworld_updateBoard() {
  if (input.b == 1) {
    input.b = 2;
    waitingTime = 0.0f;
    
    if (player.vehicle != 0) {
      uiConsole_addMessage(ultimaStrings[136]);
      uiConsole_addMessage(ultimaStrings[137]);
      return true;
    }

    int vehicleTile = vehiclesMap[player.ty][player.tx];
    if (vehicleTile == 0 || vehicleTile > 7) {
      uiConsole_addMessage(ultimaStrings[138]);
      return true;
    }

    if (vehicleTile == 1) {
      player.vehicle = 1;
      uiConsole_addMessage(ultimaStrings[139]);
    } else if (vehicleTile == 2) {
      player.vehicle = 2;
      uiConsole_addMessage(ultimaStrings[140]);
    } else if (vehicleTile == 3 || vehicleTile == 4 || vehicleTile == 5) {
      player.vehicle = vehicleTile;
      uiConsole_addMessageFormat("%.15s%.15s", ultimaStrings[144], vehicleNames[vehicleTile]);
    } else if (vehicleTile == 6) {
      player.vehicle = 6;
      uiConsole_addMessage(ultimaStrings[141]);
    } else if (vehicleTile == 7) {
      player.vehicle = 7;
      uiConsole_addMessage(ultimaStrings[142]);
    }

    vehiclesMap[player.ty][player.tx] = 0;
    playerOverworld_updateGeometry();

    return true;
  }

  return false;
}

static bool playerOverworld_updateExit() {
  if (input.x == 1) {
    input.x = 2;
    waitingTime = 0.0f;

    if (player.vehicle == 0) {
      uiConsole_addMessage(ultimaStrings[233]);
      return true;
    }

    int tile = (worldMap_getPlayerTile() >> 4) & 0xFF;
    int vehicleTile = vehiclesMap[player.ty][player.tx];
    if (tile > 2 || vehicleTile > 0) {
      uiConsole_addMessage(ultimaStrings[234]);
      uiConsole_addMessage(ultimaStrings[235]);
      return true;
    }

    vehiclesMap[player.ty][player.tx] = player.vehicle;
    
    if (player.vehicle == 1 || player.vehicle == 2) {
      uiConsole_addMessage(ultimaStrings[239]);
    } else {
      uiConsole_addMessage(ultimaStrings[240]);
    }

    player.vehicle = 0;
    playerOverworld_updateGeometry();

    return true;
  
  }
  return false;
}

static bool playerOverworld_updateFiring() {
  if (input.f == 1) {
    input.f = 2;
    waitingTime = 0.0f;
    lagTime = 1.5f;

    if (player.vehicle == 4) {
      uiConsole_replaceLastMessageFormat("%.15s%.15s", ultimaStrings[98], ultimaStrings[166]);
    } else if (player.vehicle == 5) {
      uiConsole_replaceLastMessageFormat("%.15s%.15s", ultimaStrings[98], ultimaStrings[167]);
    } else {
      uiConsole_replaceLastMessageFormat("%.15s%.15s", ultimaStrings[98], ultimaStrings[168]);
      return true;
    }

    if (enemyEncounter.monsterId < 6 || enemyEncounter.monsterId > 20) {
      uiConsole_addMessageFormat("%.15s%.15s", ultimaStrings[98], ultimaStrings[169]);
      return true;
    }

    uiConsole_queueMessageFormat("%.15s%.15s", ultimaStrings[170], enemyDefinitions[enemyEncounter.monsterId].name);

    if (rand01() > 0.8f) {
      uiConsole_queueMessage(ultimaStrings[171]);
      return true;
    }

    int damage = (int)(rand01() * 10 * player.vehicle) + 30;
    enemyEncounter.hp -= damage;

    uiConsole_queueMessageFormat("%.18s%d", ultimaStrings[172], damage);

    playerOverworld_checkIfEnemiesDead();

    return true;
  }

  return false;
}

static void playerOverworld_enterSignpost() {
  int tx = (int)(player.tx % OS_BTERRA_MAP_WIDTH);
  int ty = (int)(player.ty % OS_BTERRA_MAP_HEIGHT);
  int world = ((int)player.ty / OS_BTERRA_MAP_HEIGHT) * 2 + ((int)player.tx / OS_BTERRA_MAP_WIDTH);
  int tileType = ultimaAssets.bterraMaps[world][ty][tx] & 0x0F;

  uiConsole_queueMessage(placesNames[world * 20 + tileType + 3]);

  if (player.quests[world * 2 + tileType] > 0 && tileType == 0) {
    player.quests[world * 2 + tileType] = -1;
    uiConsole_queueMessageFormat("^T2%.27s", ultimaStrings[271]);
  }

  int postNumber = world * 2 + tileType + 1;
  int statToIncrease = -1;
  if (postNumber == 1) {
    uiConsole_queueMessageFormat("^T2%.27s", ultimaStrings[272]);
    uiConsole_queueMessageFormat("^T2%.27s", ultimaStrings[273]);
    statToIncrease = 6;
  } else if (postNumber == 2) {
    uiConsole_queueMessageFormat("^T2%.27s", ultimaStrings[274]);
    uiConsole_queueMessageFormat("^T2%.27s", ultimaStrings[275]);
    statToIncrease = 2;
  } else if (postNumber == 3) {
    for (int i=0;i<8;i++) {
      uiConsole_queueMessageFormat("^T2%.27s", ultimaStrings[276 + i]);
    }
    statToIncrease = 5;
  } else if (postNumber == 4) {
    uiConsole_queueMessageFormat("^T2%.27s", ultimaStrings[284]);
    uiConsole_queueMessageFormat("^T2%.27s", ultimaStrings[285]);

    if (lastSignpost == world * 2 + tileType) {
      uiConsole_queueMessageFormat("^T2%.27s", ultimaStrings[286]);
      return;
    }

    lastSignpost = world * 2 + tileType;

    for (int i=0;i<OS_WEAPONS_COUNT;i++) {
      if (player.weapons[i] == 0) {
        uiConsole_queueMessageFormat("^T2%.27s", ultimaStrings[288]);
        uiConsole_queueMessageFormat("^T2%.27s", weaponNames[i + 1]);
        uiConsole_queueMessageFormat("^T2%.27s", ultimaStrings[289]);

        player.weapons[i] = 1;
        return;
      }
    }

    uiConsole_queueMessageFormat("^T2%.27s", ultimaStrings[286]);
    return;
  } else if (postNumber == 5) {
    for (int i=0;i<5;i++){
      uiConsole_queueMessageFormat("^T2%.27s", ultimaStrings[290 + i]);
    }
    statToIncrease = 3;
  } else if (postNumber == 6) {
    for (int i=0;i<3;i++){
      uiConsole_queueMessageFormat("^T2%.27s", ultimaStrings[295 + i]);
    }
    statToIncrease = 4;
  } else if (postNumber == 7) {
    for (int i=0;i<2;i++){
      uiConsole_queueMessageFormat("^T2%.27s", ultimaStrings[298 + i]);
    }
    statToIncrease = 3;
  } else if (postNumber == 8) {
    for (int i=0;i<2;i++){
      uiConsole_queueMessageFormat("^T2%.27s", ultimaStrings[300 + i]);
    }
    statToIncrease = 3;
  }

  if (lastSignpost == world * 2 + tileType) {
    uiConsole_queueMessageFormat("^T2%.27s", ultimaStrings[286]);
    return;
  }

  lastSignpost = world * 2 + tileType;
  int statIncrase = (int)((99 - *(&player.health + statToIncrease)) / 10);
  uiConsole_queueMessageFormat("^T2%.15s+%d", statsNames[statToIncrease], statIncrase);
  (*(&player.health + statToIncrease)) += statIncrase;

  uiConsole_updateStats();
}

static bool playerOverworld_updateEnter() {
  if (input.e == 1) {
    input.e = 2;
    waitingTime = 0.0f;

    int tx = (int)(player.tx % OS_BTERRA_MAP_WIDTH);
    int ty = (int)(player.ty % OS_BTERRA_MAP_HEIGHT);
    int world = ((int)player.ty / OS_BTERRA_MAP_HEIGHT) * 2 + ((int)player.tx / OS_BTERRA_MAP_WIDTH);
    int tile = (ultimaAssets.bterraMaps[world][ty][tx] >> 4) & 0x0F;
    int tileType = ultimaAssets.bterraMaps[world][ty][tx] & 0x0F;

    if (tile < 4 || tile > 7) {
      uiConsole_replaceLastMessageFormat("%.10s%.10s", ultimaStrings[98], ultimaStrings[163]);
      uiConsole_addMessage(ultimaStrings[164]);
      return true;
    }

    uiConsole_replaceLastMessageFormat("%.15s%.15s", ultimaStrings[98], ultimaStrings[163]);

    if (tile == 5) {
      playerOverworld_enterSignpost();

      return true;
    } else if (tile == 6) {
      uiConsole_queueMessageFormat("%s%s", ultimaStrings[180], placesNames[world * 20 + tileType + 13]);
      uiConsole_queueMessageFormat(" ");
      scene_load(&sceneTown);
      return true;
    }
  }
  return false;
}

void playerOverworld_setCameraFollow() {
  camera_setPosition3f(&camera, (player.tx + 1) * OS_TILE_WIDTH - OS_SCREEN_WIDTH / 2, (player.ty + 1) * OS_TILE_HEIGHT - OS_SCREEN_HEIGHT / 2, 10);
}

bool playerOverworld_update(float deltaTime) {
  bool acted = false;

  if (keyRepeatDelay <= 0) {
    switch (playerState) {
      case PLAYER_STATE_IDLE:
        if (playerOverworld_updateZtats()) { acted = true; } else
        if (playerOverworld_updateWait()) { acted = true; } else
        if (playerOverworld_updateSave()) { acted = true; } else 
        if (playerOverworld_updateInfo()) { acted = true; } else
        if (playerOverworld_get()) { acted = true; } else
        if (playerOverworld_open()) { acted = true; } else
        if (playerOverworld_drop()) { acted = true; } else
        if (playerOverworld_ready()) { acted = true; } else
        if (playerOverworld_updateAttack()) { acted = true; } else
        if (playerOverwolrd_cast()) { acted = true; } else
        if (playerOverworld_updateBoard()) { acted = true; } else
        if (playerOverworld_updateExit()) { acted = true; } else
        if (playerOverworld_updateFiring()) { acted = true; } else
        if (playerOverworld_updateEnter()) { acted = true; } else
        if (playerOverworld_updateMovement(deltaTime)) { acted = true; }
        break;
      case PLAYER_STATE_READY_TYPE:
        if (playerOverworld_ready()) { acted = true; }
        break;
    }
  } else {
    keyRepeatDelay -= deltaTime;
    if (keyRepeatDelay < 0) {
      keyRepeatDelay = 0;
    }
  }

  if (acted) {
    player.time += 0.6f;
  }

  return acted;
}

void playerOverworld_render() {
  matrix4_setPosition(transformationMatrix, player.tx * OS_TILE_WIDTH, player.ty * OS_TILE_HEIGHT, 2);
  float *viewMatrix = camera_getViewProjectionMatrix(&camera);

  geometry_render(&playerOverworldGeometry, ultimaAssets.overworldTiles.textureId, transformationMatrix, viewMatrix);

  if (renderEnemy && enemyGeometry != NULL) {
    geometry_render(enemyGeometry, ultimaAssets.enemySprites.textureId, enemyTransformationMatrix, viewMatrix);
  }
}

void playerOverworld_free() {
  geometry_free(&playerOverworldGeometry);
  if (enemyGeometry != NULL) {
    geometry_free(enemyGeometry);
    free(enemyGeometry);
    enemyGeometry = NULL;
  }
}