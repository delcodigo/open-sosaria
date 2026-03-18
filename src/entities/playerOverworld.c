#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "playerOverworld.h"
#include "engine/geometry.h"
#include "engine/camera.h"
#include "engine/input.h"
#include "data/saveAndLoad.h"
#include "data/bevery.h"
#include "ui/uiConsole.h"
#include "ui/uiztats.h"
#include "scenes/sceneDiskLoader.h"
#include "maths/matrix4.h"
#include "config.h"

static PLAYER_STATE playerState = PLAYER_STATE_IDLE;
static Geometry playerOverworldGeometry;
static float transformationMatrix[16];
static int readyStep = 0;
static Textfield inputTextfieldBuffer;
static char selectedWeapon[3] = {0};
static bool areKeysReleased = true;

static float keyRepeatDelay = 0;
static float waitingTime = 0.0f;

void playerOverworld_init() {
  geometry_setSprite(&playerOverworldGeometry, OS_TILE_WIDTH, OS_TILE_HEIGHT, 0, 0.5f, 0.125f, 1.0f);
  matrix4_setIdentity(transformationMatrix);
  memset(&inputTextfieldBuffer, 0, sizeof(inputTextfieldBuffer));
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
    int tx = (int)((player.tx + moveX) % OS_BTERRA_MAP_WIDTH);
    int ty = (int)((player.ty + moveY) % OS_BTERRA_MAP_HEIGHT);
    int world = ((int)(player.ty + moveY) / OS_BTERRA_MAP_HEIGHT) * 2 + ((int)(player.tx + moveX) / OS_BTERRA_MAP_WIDTH);
    int tile = (ultimaAssets.bterraMaps[world][ty][tx] >> 4) & 0x0F;

    uiConsole_replaceLastMessage(movementCommand);

    if (tile == 0) {
      uiConsole_addMessage(ultimaStrings[122]);
      keyRepeatDelay = 0.3f;
      return true;
    } else if (tile == 3) {
      uiConsole_addMessage(ultimaStrings[123]);
      keyRepeatDelay = 0.3f;
      return true;
    }

    player.tx = (player.tx + moveX + OS_BTERRA_MAP_WIDTH * 2) % (OS_BTERRA_MAP_WIDTH * 2);
    player.ty = (player.ty + moveY + OS_BTERRA_MAP_HEIGHT * 2) % (OS_BTERRA_MAP_HEIGHT * 2);
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
      char placeName[41] = {0};
      snprintf(placeName, sizeof(placeName), "%s%s", ultimaStrings[180], placesNames[world * 20 + tileType + 13]);
      uiConsole_addMessage(placeName);
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
  char consoleMessage[31] = {0};

  if (spellIndex < 0) {
    snprintf(consoleMessage, sizeof(consoleMessage), "%.15s%s%.10s", ultimaStrings[214], charUsed, ultimaStrings[224]);
    uiConsole_replaceLastMessage(consoleMessage);
    return;
  }

  if (player.spells[spellIndex - 1] < 1 && spellIndex > 0) {
    snprintf(consoleMessage, sizeof(consoleMessage), "%.15s%s", ultimaStrings[225], spellNames[spellIndex]);
    uiConsole_addMessage(consoleMessage);
    return;
  }

  player.spell = spellIndex;
  snprintf(consoleMessage, sizeof(consoleMessage), "%.15s%s", ultimaStrings[226], spellNames[spellIndex]);
  uiConsole_addMessage(consoleMessage);
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

  if (readyStep == 0){
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
      readyStep = 1;

      return false;
    }
  } else if (readyStep == 1) {
    if (input.w != 0 && input.a != 0 && input.s != 0) { return false; }

    if (input.w == 1) {
      input.w = 2;
      memset(selectedWeapon, 0, sizeof(selectedWeapon));
      uiConsole_addMessage(ultimaStrings[205]);
      playerOverworld_activateInput();
      readyStep = 2;
    } else if (input.a == 1) {
      input.a = 2;
      uiConsole_addMessage(ultimaStrings[210]);
      playerOverworld_activateInput();
      readyStep = 4;
    } else if (input.s == 1) {
      input.s = 2;
      uiConsole_addMessage(ultimaStrings[214]);
      playerOverworld_activateInput();
      readyStep = 5;
    }
    
    return false;
  } else if (readyStep == 2) { // Weapon: select two letter weapon abbreviation
    if (inputTextfield->isDirty && inputTextfield->text[0] != '\0') {
      inputTextfield->isDirty = false;

      if (selectedWeapon[0] == '\0') {
        selectedWeapon[0] = (char)toupper(inputTextfield->text[0]);
      } else if (selectedWeapon[1] == '\0') {
        selectedWeapon[1] = (char)toupper(inputTextfield->text[0]);
        readyStep = 3;
      }

      char weaponCommand[31] = {0};
      snprintf(weaponCommand, sizeof(weaponCommand), "%.14s%.2s", ultimaStrings[205], selectedWeapon);
      uiConsole_replaceLastMessage(weaponCommand);

      inputTextfield->cursorPosition = 0;
      inputTextfield->text[0] = '\0';
      areKeysReleased = false;

      return false;
    }
  } else if (readyStep == 3) { // Weapon: equip selected weapon
    char weaponCommand[31] = {0};
    readyStep = 7;
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
  } else if (readyStep == 4) { // Armor: select and equip armor
    if (inputTextfield->isDirty && inputTextfield->text[0] != '\0') {
      inputTextfield->isDirty = false;
      char consoleMessage[31] = {0};
      char armorChar[2] = {(char)toupper(inputTextfield->text[0]), '\0'};
      snprintf(consoleMessage, sizeof(consoleMessage), "%.14s%.1s", ultimaStrings[210], armorChar);
      uiConsole_replaceLastMessage(consoleMessage);

      readyStep = 7;
      areKeysReleased = false;

      memset(consoleMessage, 0, sizeof(consoleMessage));

      for (int i=0;i<OS_ARMORS_COUNT;i++) {
        if (armorNames[i][0] == armorChar[0]) {
          if (player.armors[i - 1] < 1 && i > 0) {
            snprintf(consoleMessage, sizeof(consoleMessage), "%.15s%.15s", ultimaStrings[212], armorNames[i]);
            uiConsole_addMessage(consoleMessage);
            inputTextfield = NULL;
            return true;
          }

          snprintf(consoleMessage, sizeof(consoleMessage), "%.15s%.15s", ultimaStrings[213], armorNames[i]);
          uiConsole_addMessage(consoleMessage);
          inputTextfield = NULL;
          player.armor = i - 1;
          return true;
        }
      }

      snprintf(consoleMessage, sizeof(consoleMessage), "%.1s%.15s", armorChar, ultimaStrings[211]);
      uiConsole_addMessage(consoleMessage);
      inputTextfield = NULL;
      return true;
    }
  } else if (readyStep == 5) { // Spells: select and equip spell
    if (inputTextfield->isDirty && inputTextfield->text[0] != '\0') {
      char selectedSpellChar[2] = {(char)toupper(inputTextfield->text[0]), '\0'};
      char consoleMessage[31] = {0};

      if (selectedSpellChar[0] == 'L') {
        snprintf(consoleMessage, sizeof(consoleMessage), "%.15sL %.10s", ultimaStrings[214], ultimaStrings[216]);
        uiConsole_replaceLastMessage(consoleMessage);
        readyStep = 6;
        areKeysReleased = false;
      } else {
        int spellIndex = -1;
        for (int i=0;i<OS_SPELLS_COUNT;i++) {
          if (spellNames[i][0] == selectedSpellChar[0]) {
            spellIndex = i;
            break;
          }
        }

        playerOverworld_tryAndEquipSpell(spellIndex, selectedSpellChar);
        readyStep = 7;
        areKeysReleased = false;

        return true;
      }
    }
  } else if (readyStep == 6) { // Select if spell is ladder up or down
    if (inputTextfield->isDirty && inputTextfield->text[0] != '\0') {
      char selectedSpellChar[3] = {'L', (char)toupper(inputTextfield->text[0]), '\0'};
      char consoleMessage[31] = {0};
      snprintf(consoleMessage, sizeof(consoleMessage), "%.15s%s", ultimaStrings[214], selectedSpellChar);
      uiConsole_replaceLastMessage(consoleMessage);

      readyStep = 7;
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
  } else if (readyStep == 7) { // Return to idle
    inputTextfield = NULL;
    inputTextfieldBuffer.active = false;
    playerState = PLAYER_STATE_IDLE;
    readyStep = 0;
  }

  return false;
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
      
  matrix4_setPosition(transformationMatrix, player.tx * OS_TILE_WIDTH, player.ty * OS_TILE_HEIGHT, 1);
  camera_setPosition3f(&camera, (player.tx + 1) * OS_TILE_WIDTH - OS_SCREEN_WIDTH / 2, (player.ty + 1) * OS_TILE_HEIGHT - OS_SCREEN_HEIGHT / 2, 10);
  float *viewMatrix = camera_getViewProjectionMatrix(&camera);

  geometry_render(&playerOverworldGeometry, ultimaAssets.overworldTiles.textureId, transformationMatrix, viewMatrix);

  return acted;
}

void playerOverworld_free() {
  geometry_free(&playerOverworldGeometry);
}