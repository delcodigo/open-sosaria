#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "playerCastle.h"
#include "engine/geometry.h"
#include "engine/input.h"
#include "playerTown.h"
#include "playerCommons.h"
#include "data/player.h"
#include "data/bevery.h"
#include "scenes/sceneDiskLoader.h"
#include "scenes/sceneCastle.h"
#include "scenes/sceneOverworld.h"
#include "maths/matrix4.h"
#include "entities/ui/uiConsole.h"
#include "config.h"
#include "utils.h"

static Geometry playerCastleGeometry;
static float transformMatrix[16];
static DROP_STEP dropStep = DROP_STEP_START;
static char itemToDrop[7];

void playerCastle_init() {
  float tx1 = (6.0f * OS_TOWN_CASTLE_SPRITE_WIDTH) / (float)ultimaAssets.townCastleSprites.width;
  float tx2 = (7.0f * OS_TOWN_CASTLE_SPRITE_WIDTH) / (float)ultimaAssets.townCastleSprites.width;

  geometry_setSprite(&playerCastleGeometry, OS_TOWN_CASTLE_SPRITE_WIDTH, OS_TOWN_CASTLE_SPRITE_HEIGHT, tx1, 0, tx2, 1);
}

static bool playerCastle_updateBoard() {
  if (input.b == 1) {
    input.b = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[665]);
    uiConsole_queueMessage(ultimaStrings[666]);

    return true;
  }

  return false;
}

static bool playerCastle_updateCast() {
  if (input.c == 1) {
    input.c = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[667]);
    uiConsole_queueMessage(ultimaStrings[668]);
    uiConsole_queueMessage(ultimaStrings[669]);

    return true;
  }

  return false;
}

static bool playerCastle_updateDrop() {
  if (dropStep == DROP_STEP_SELECT_GOLD) {
    if (lastKey >= GLFW_KEY_0 && lastKey <= GLFW_KEY_9) {
      if (lastKey == GLFW_KEY_0 && itemToDrop[0] == '\0') {
        return false;
      }

      int len = strlen(itemToDrop);
      if (len < 6) {
        itemToDrop[len] = lastKey;
        itemToDrop[len + 1] = '\0';
        lastKey = 0;
        uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[369], itemToDrop);
      } else {
        uiConsole_queueMessage(ultimaStrings[370]);
        playerState = PLAYER_STATE_IDLE;
        dropStep = DROP_STEP_START;
        return true;
      }
    } else if (lastKey == GLFW_KEY_ENTER && itemToDrop[0] != '\0') {
      int value = atoi(itemToDrop);
      if (value > player.gold) {
        uiConsole_queueMessageFormat("%s%d", ultimaStrings[371], value);
        playerState = PLAYER_STATE_IDLE;
        dropStep = DROP_STEP_START;
        return true;
      }

      uiConsole_queueMessage(ultimaStrings[680]);
      player.gold -= value;

      if (player.px < 12 && player.px > 3 && player.py > 6 && player.py < 15) {
        uiConsole_queueMessage(ultimaStrings[681]);
        int attributeId = (int)(rand01() * 7);
        *((&player.health) + attributeId) += (int)(value / 10.0f);
        if (*((&player.health) + attributeId) > 99) {
          *((&player.health) + attributeId) = 99;
        }
      }

      if (player.px < 27 && player.px > 21 && player.py > 5 && player.py < 11) {
        uiConsole_queueMessage(ultimaStrings[682]);
        int weaponId = (int)(rand01() * 15);
        player.weapons[weaponId] += ((int)(value / 10.0f)) > 0 ? 1 : 0;
      }

      if (player.px < 27 && player.px > 21 && player.py > 10 && player.py < 16) {
        uiConsole_queueMessage(ultimaStrings[683]);
        player.food += value * 5.0f;
      }

      uiConsole_updateStats();

      playerState = PLAYER_STATE_IDLE;
      dropStep = DROP_STEP_START;
      return true;
    }
  } else if (dropStep == DROP_STEP_SELECT_WEAPON) {
    if (lastKey >= GLFW_KEY_A && lastKey <= GLFW_KEY_Z) {
      if (itemToDrop[0] == '\0') {
        itemToDrop[0] = lastKey;
        lastKey = 0;
        uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[375], itemToDrop);
      } else if (itemToDrop[1] == '\0') {
        itemToDrop[1] = lastKey;
        lastKey = 0;
        uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[375], itemToDrop);

        int selectedWeaponId = -1;
        for (int i=0;i<OS_WEAPONS_COUNT;i++) {
          if (weaponNames[i + 1][0] == itemToDrop[0] && weaponNames[i + 1][1] == itemToDrop[1]) {
            selectedWeaponId = i;
            break;
          }
        }

        if (selectedWeaponId == -1) {
          uiConsole_queueMessageFormat("%s%s", itemToDrop, ultimaStrings[376]);
        } else if (player.weapons[selectedWeaponId] < 1) {
          uiConsole_queueMessageFormat("%s%s", ultimaStrings[377], weaponNames[selectedWeaponId + 1]);
        } else {
          player.weapons[selectedWeaponId]--;
          if (player.weapons[selectedWeaponId] == 0) {
            player.weapon = 0;
          }

          uiConsole_queueMessage(ultimaStrings[378]);
        }

        playerState = PLAYER_STATE_IDLE;
        dropStep = DROP_STEP_START;
        return true;
      }
    }
  } else if (dropStep == DROP_STEP_SELECT_ARMOR) {
    if (lastKey >= GLFW_KEY_A && lastKey <= GLFW_KEY_Z) {
      int selectedArmorId = -1;
      for (int i=0;i<OS_ARMORS_COUNT;i++) {
        if (armorNames[i + 1][0] == lastKey) {
          selectedArmorId = i;
          break;
        }
      }

      uiConsole_replaceLastMessageFormat("%s%c", ultimaStrings[379], lastKey);
      
      if (selectedArmorId == -1) {
        uiConsole_queueMessageFormat("%c%s", lastKey, ultimaStrings[380]);
      } else if (player.armors[selectedArmorId] < 1) {
        uiConsole_queueMessage(ultimaStrings[381]);
        uiConsole_queueMessage(armorNames[selectedArmorId + 1]);
      } else {
        player.armors[selectedArmorId]--;
        if (player.armors[selectedArmorId] == 0) {
          player.armor = 0;
        }

        uiConsole_queueMessage(ultimaStrings[382]);
      }

      lastKey = 0;
      playerState = PLAYER_STATE_IDLE;
      dropStep = DROP_STEP_START;
      return true;
    }
  } else if (playerState == PLAYER_STATE_DROP) {
    if (lastKey != 0){
      if (lastKey == GLFW_KEY_G) {
        uiConsole_replaceLastMessageFormat("%s GOLD", ultimaStrings[363]);
        uiConsole_queueMessage(ultimaStrings[369]);
        dropStep = DROP_STEP_SELECT_GOLD;
        memset(itemToDrop, 0, sizeof(itemToDrop));
        lastKey = 0;
      } else if (lastKey == GLFW_KEY_W) {
        uiConsole_replaceLastMessageFormat("%s WEAPON", ultimaStrings[363]);
        uiConsole_queueMessage(ultimaStrings[375]);
        dropStep = DROP_STEP_SELECT_WEAPON;
        memset(itemToDrop, 0, sizeof(itemToDrop));
        lastKey = 0;
      } else if (lastKey == GLFW_KEY_A) {
        uiConsole_replaceLastMessageFormat("%s ARMOR", ultimaStrings[363]);
        uiConsole_queueMessage(ultimaStrings[379]);
        dropStep = DROP_STEP_SELECT_ARMOR;
        lastKey = 0;
      } else {
        uiConsole_queueMessage(ultimaStrings[367]);
        playerState = PLAYER_STATE_IDLE;
        dropStep = DROP_STEP_START;
        return true;
      }
    }
  } else if (input.d == 1) {
    input.d = 2;
    lastKey = 0;

    uiConsole_replaceLastMessageFormat("%.14s%.15s", ultimaStrings[98], ultimaStrings[362]);
    uiConsole_queueMessage(ultimaStrings[363]);

    playerState = PLAYER_STATE_DROP;
    dropStep = DROP_STEP_START;
  }

  return false;
}

static bool playerCastle_updateEnter() {
  if (input.e == 1) {
    input.e = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[693]);
    uiConsole_queueMessage(ultimaStrings[694]);

    return true;
  }

  return false;
}

static bool playerCastle_updateFiring() {
  if (input.f == 1) {
    input.f = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[695]);
    uiConsole_queueMessage(ultimaStrings[696]);

    return true;
  }

  return false;
}

static bool playerCastle_updateGet() {
  if (input.g == 1) {
    input.g = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[697]);

    if (allowToTakeItemsFromCastle < 1) {
      uiConsole_queueMessage(ultimaStrings[699]);
      uiConsole_queueMessage(ultimaStrings[700]);
      return true;
    }

    if (player.px > 15 && player.px < 26 && player.py < 5) {
      int armorIndex = (int)(rand01() * OS_ARMORS_COUNT);
      player.armors[armorIndex]++;
      uiConsole_queueMessage(ultimaStrings[701]);
      uiConsole_queueMessage(armorNames[armorIndex + 1]);
      allowToTakeItemsFromCastle--;
    } else if (player.px > 15 && player.px < 26 && player.py > 16) {
      int weaponIndex = (int)(rand01() * OS_WEAPONS_COUNT);
      player.weapons[weaponIndex]++;
      uiConsole_queueMessage(ultimaStrings[702]);
      uiConsole_queueMessage(weaponNames[weaponIndex + 1]);
      allowToTakeItemsFromCastle--;
    } else if (player.px > 26 && player.px < 31 && player.py < 5) {
      int foodObtained = (int)(pow(rand01(), 2) * 15) + 1;
      player.food += foodObtained;
      uiConsole_queueMessage(ultimaStrings[703]);
      uiConsole_queueMessageFormat("%d%s", foodObtained, ultimaStrings[704]);
      allowToTakeItemsFromCastle--;
    } else {
      uiConsole_queueMessage(ultimaStrings[698]);
    }

    return true;
  }

  return false;
}

static bool playerCastle_updateInfo() {
  if (input.i == 1) {
    input.i = 2;
    waitingTime = 0.0f;

    int tx = (int)(player.tx % OS_BTERRA_MAP_WIDTH);
    int ty = (int)(player.ty % OS_BTERRA_MAP_HEIGHT);
    int world = ((int)player.ty / OS_BTERRA_MAP_HEIGHT) * 2 + ((int)player.tx / OS_BTERRA_MAP_WIDTH);
    int tileType = ultimaAssets.bterraMaps[world][ty][tx] & 0x0F;

    uiConsole_replaceLastMessageFormat("%.14s%.15s", ultimaStrings[98], ultimaStrings[707]);
    uiConsole_queueMessage(ultimaStrings[708]);
    uiConsole_queueMessage(placesNames[world * 20 + tileType + 1]);
    uiConsole_queueMessage(ultimaStrings[709]);

    return true;
  }

  return false;
}

static bool playerCastle_updateOpen() {
  if (input.o == 1) {
    input.o = 2;
    waitingTime = 0.0f;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[715]);

    return true;
  }

  return false;
}

static bool playerCastle_updateSave() {
  if (input.q == 1) {
    input.q = 2;
    uiConsole_replaceLastMessageFormat("%.14s%.15s", ultimaStrings[98], ultimaStrings[717]);
    uiConsole_queueMessage(ultimaStrings[718]);
    return true;
  }

  return false;
}

static bool playerCastle_updateSteal() {
  if (input.s == 1) {
    input.s = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[736]);

    if (rand01() > 0.8f || enemyEncounter.monsterId > 0 || (rand01() > 0.8f && player.type != 4)) {
      uiConsole_queueMessage(ultimaStrings[738]);
      enemyEncounter.monsterId = 1;
      return true;
    }

    if (player.px > 15 && player.px < 26 && player.py < 5) {
      int armorIndex = (int)(rand01() * OS_ARMORS_COUNT);
      player.armors[armorIndex]++;
      uiConsole_queueMessage(ultimaStrings[739]);
      uiConsole_queueMessage(armorNames[armorIndex + 1]);
    } else if (player.px > 15 && player.px < 26 && player.py > 16) {
      int weaponIndex = (int)(rand01() * OS_WEAPONS_COUNT);
      player.weapons[weaponIndex]++;
      uiConsole_queueMessage(ultimaStrings[740]);
      uiConsole_queueMessage(weaponNames[weaponIndex + 1]);
    } else if (player.px > 26 && player.px < 31 && player.py < 5) {
      int foodObtained = (int)(rand01() * 30) + 1;
      player.food += foodObtained;
      uiConsole_queueMessage(ultimaStrings[741]);
      uiConsole_queueMessageFormat("%d%s", foodObtained, ultimaStrings[742]);
    } else {
      uiConsole_queueMessage(ultimaStrings[737]);
    }

    return true;
  }

  return false;
}

bool playerCastle_updateExit() {
  if (input.x == 1) {
    input.x = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[801]);
    uiConsole_queueMessage(ultimaStrings[802]);

    return true;
  }

  return false;
}

bool playerCastle_update(float deltaTime) {
  bool acted = false;
  
  if (keyRepeatDelay <= 0) {
    switch (playerState) {
      case PLAYER_STATE_IDLE:
        if (playerCommons_updateZtats()) { acted = true; } else
        if (playerCommons_updateReady()) { acted = true; } else
        if (playerCommons_updateWait()) { acted = true; } else
        if (playerCastle_updateBoard()) { acted = true; } else
        if (playerCastle_updateCast()) { acted = true; } else
        if (playerCastle_updateDrop()) { acted = true; } else
        if (playerCastle_updateEnter()) { acted = true; } else
        if (playerCastle_updateFiring()) { acted = true; } else
        if (playerCastle_updateGet()) { acted = true; } else
        if (playerCastle_updateInfo()) { acted = true; } else
        if (playerCastle_updateOpen()) { acted = true; } else
        if (playerCastle_updateSave()) { acted = true; } else
        if (playerCastle_updateSteal()) { acted = true; } else
        if (playerCastle_updateExit()) { acted = true; } else
        if (playerTown_updateAttack()) { acted = true; } else
        if (playerTown_updateMovement(deltaTime)) { acted = true; }
        break;

      case PLAYER_STATE_READY_TYPE:
        if (playerCommons_updateReady()) { acted = true; }
        break;
      
      case PLAYER_STATE_DROP:
        if (playerCastle_updateDrop()) { acted = true; }
        break;

      case PLAYER_STATE_TOWN_ATTACK:
        if (playerTown_updateAttack()) { acted = true; }
        break;

      default:
        break;
    }
  } else {
    keyRepeatDelay -= deltaTime;
    if (keyRepeatDelay < 0) {
      keyRepeatDelay = 0;
    }
  }

  return acted;
}

void playerCastle_render(float *viewMatrix) {
  matrix4_setIdentity(transformMatrix);
  matrix4_setPosition(transformMatrix, player.px * OS_TOWN_CASTLE_SPRITE_WIDTH, player.py * OS_TOWN_CASTLE_SPRITE_HEIGHT, 1);

  geometry_render(&playerCastleGeometry, ultimaAssets.townCastleSprites.textureId, transformMatrix, viewMatrix);
}

void playerCastle_free() {
  geometry_free(&playerCastleGeometry);
}