#include <stdlib.h>
#include <string.h>
#include "playerCastle.h"
#include "engine/geometry.h"
#include "engine/input.h"
#include "playerTown.h"
#include "playerCommons.h"
#include "data/player.h"
#include "data/bevery.h"
#include "scenes/sceneDiskLoader.h"
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
        if (playerTown_updateMovement(deltaTime)) { acted = true; }
        break;

      case PLAYER_STATE_READY_TYPE:
        if (playerCommons_updateReady()) { acted = true; }
        break;
      
      case PLAYER_STATE_DROP:
        if (playerCastle_updateDrop()) { acted = true; }
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