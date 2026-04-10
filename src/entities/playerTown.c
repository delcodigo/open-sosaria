#include <stdlib.h>
#include <string.h>
#include "playerTown.h"
#include "engine/geometry.h"
#include "engine/input.h"
#include "entities/ui/uiConsole.h"
#include "scenes/sceneDiskLoader.h"
#include "scenes/sceneOverworld.h"
#include "scenes/sceneTown.h"
#include "scenes/sceneCastle.h"
#include "merchantTown.h"
#include "data/player.h"
#include "data/bevery.h"
#include "maths/matrix4.h"
#include "playerCommons.h"
#include "vehicleOverworld.h"
#include "vmExecuter.h"
#include "config.h"

static Geometry playerTownGeometry;
static float transformMatrix[16];
static DROP_STEP dropStep = DROP_STEP_START;
static char itemToDrop[7];
static bool droppedGold = false;

void playerTown_init() {
  float tx1 = (6.0f * OS_TOWN_CASTLE_SPRITE_WIDTH) / (float)ultimaAssets.townCastleSprites.width;
  float tx2 = (7.0f * OS_TOWN_CASTLE_SPRITE_WIDTH) / (float)ultimaAssets.townCastleSprites.width;

  geometry_setSprite(&playerTownGeometry, OS_TOWN_CASTLE_SPRITE_WIDTH, OS_TOWN_CASTLE_SPRITE_HEIGHT, tx1, 0, tx2, 1);
  
  enemyEncounter.monsterId = 0;
  droppedGold = false;
}

static bool playerTown_checkExit(int moveX, int moveY) {
  if (isPlayerInCastle && player.px + moveX < 0) {
    if (princessPosition.x < 5 && princessPosition.y > 0) {
      uiConsole_queueMessageFormat("^T1%s", ultimaStrings[636]);
      uiConsole_queueMessageFormat("^T1%s", ultimaStrings[637]);
      uiConsole_queueMessageFormat("^T1%s", ultimaStrings[638]);

      player.health += 3000;
      player.gold += 3000;
      player.experience += 3000;

      uiConsole_updateStats();
    }

    if (princessPosition.x < 5 && princessPosition.y > 0 && player.time > 7000 && player.spaceLevel > 20) {
      uiConsole_queueMessageFormat("^T1%s", ultimaStrings[639]);
      uiConsole_queueMessageFormat("^T1%s", ultimaStrings[640]);
      uiConsole_queueMessageFormat("^T1%s", ultimaStrings[641]);
      uiConsole_queueMessageFormat("^T1%s", ultimaStrings[642]);
      uiConsole_queueMessageFormat("^T1%s", ultimaStrings[643]);
      uiConsole_queueMessageFormat("^T1%s", ultimaStrings[644]);

      vehiclesMap[3][3] = 7;
    }

    enemyEncounter.monsterId = -1;
    vmExecuter_createSceneTransition(0.5f, &sceneOverworld);
    return true;
  }

  if (!isPlayerInCastle && player.py + moveY > 21) {
    vmExecuter_createSceneTransition(0.5f, &sceneOverworld);
    return true;
  }

  return false;
}

static bool playerTown_isSolid(int x, int y) {
  if (isPlayerInCastle) {
    return sceneCastle_isSolid(x, y);
  } else {
    return sceneTown_isSolid(x, y);
  }
}

bool playerTown_updateMovement(float deltaTime) {
  int moveX = 0;
  int moveY = 0;
  int movementStringIndex = -1;

  if (input.up) {
    moveY = -1;
    movementStringIndex = 117;
  } else if (input.down) {
    moveY = 1;
    movementStringIndex = 118;
  } else if (input.left) {
    moveX = -1;
    movementStringIndex = 120;
  } else if (input.right) {
    moveX = 1;
    movementStringIndex = 119;
  }

  if (moveX != 0 || moveY != 0) {
    waitingTime = 0.0f;
    uiConsole_replaceLastMessageFormat("%.14s%.15s", ultimaStrings[98], ultimaStrings[movementStringIndex]);

    if (playerTown_isSolid(player.px + moveX, player.py + moveY)) {
      uiConsole_addMessage(ultimaStrings[341]);
      keyRepeatDelay = 0.3f;
      return true;
    }
    
    if (playerTown_checkExit(moveX, moveY)) {
      return true;
    }

    player.px = player.px + moveX;
    player.py = player.py + moveY;
    keyRepeatDelay = 0.1f;

    player_consumeTownFood();

    return true;
  } else {
    waitingTime += deltaTime;
    if (waitingTime >= 5.0f) {
      waitingTime = 0.0f;
      uiConsole_replaceLastMessageFormat("%.14s%.15s", ultimaStrings[98], ultimaStrings[99]);
      player_waitPenalty();
      return true;
    }
  }
  
  return false;
}

static bool playerTown_updateCast() {
  if (input.c == 1) {
    input.c = 2;
    waitingTime = 0.0f;

    uiConsole_replaceLastMessageFormat("%.15s%.15s", ultimaStrings[98], ultimaStrings[359]);
    uiConsole_queueMessage(ultimaStrings[360]);
    uiConsole_queueMessage(ultimaStrings[361]);

    return true;
  }

  return false;
}

static bool playerTown_updateGet() {
  if (input.g == 1) {
    input.g = 2;
    waitingTime = 0.0f;

    uiConsole_replaceLastMessageFormat("%.14s%.15s", ultimaStrings[98], ultimaStrings[383]);

    return true;
  }

  return false;
}

static bool playerTown_updateInfo() {
  if (input.i == 1) {
    input.i = 2;
    waitingTime = 0.0f;

    int tx = (int)(player.tx % OS_BTERRA_MAP_WIDTH);
    int ty = (int)(player.ty % OS_BTERRA_MAP_HEIGHT);
    int world = ((int)player.ty / OS_BTERRA_MAP_HEIGHT) * 2 + ((int)player.tx / OS_BTERRA_MAP_WIDTH);
    int tileType = ultimaAssets.bterraMaps[world][ty][tx] & 0x0F;

    uiConsole_replaceLastMessageFormat("%.14s%.15s", ultimaStrings[98], ultimaStrings[384]);
    uiConsole_queueMessage(ultimaStrings[385]);
    uiConsole_queueMessageFormat("%.15s%.15s", ultimaStrings[386], placesNames[world * 20 + tileType + 13]);
    uiConsole_queueMessage(ultimaStrings[387]);

    return true;
  }

  return false;
}

static bool playerTown_updateSave() {
  if (input.q == 1) {
    input.q = 2;
    uiConsole_replaceLastMessageFormat("%.14s%.15s", ultimaStrings[98], ultimaStrings[389]);
    uiConsole_queueMessage(ultimaStrings[390]);
    return true;
  }

  return false;
}

static bool playerTown_updateDrop() {
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

      if (player.px >= 29 && player.py >= 13) {
        uiConsole_queueMessage(ultimaStrings[372]);
        player.health += (int)(value * 1.5f);

        if (!droppedGold) {
          player.weapons[0] += 4;
          droppedGold = true;
        }
      }

      player.gold -= value;
      uiConsole_queueMessage(ultimaStrings[373]);

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

static void playerTown_attackAt(int x, int y) {
  if (isPlayerInCastle) {
    sceneCastle_attackAt(x, y);
  } else {
    sceneTown_attackAt(x, y);
  }
}

bool playerTown_updateAttack() {
  if (playerState == PLAYER_STATE_TOWN_ATTACK) {
    if (lastKey == 0) { return false; }

    int dx = 0;
    int dy = 0;

    if (input.up) {
      dy = -1;
    } else if (input.down) {
      dy = 1;
    } else if (input.left) {
      dx = -1;
    } else if (input.right) {
      dx = 1;
    } else {
      uiConsole_queueMessage(ultimaStrings[343]);
      return false;
    }

    int range = 1;
    if (player.weapon >= 6 && player.weapon != 12) {
      range = 5;
    }

    for (int i=1;i<=range;i++) {
      if (player.py + dy * i >= 22) {
        break;
      }

      if (playerTown_isSolid(player.px + dx * i, player.py + dy * i)) {
        dx *= i;
        dy *= i;
        break;
      }
    }

    playerTown_attackAt(player.px + dx, player.py + dy);
    playerState = PLAYER_STATE_IDLE;

    return true;
  } else if (input.a == 1) {
    input.a = 2;
    waitingTime = 0.0f;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[342]);
    lastKey = 0;
    playerState = PLAYER_STATE_TOWN_ATTACK;
  }

  return false;
}

bool playerTown_update(float deltaTime) {
  bool acted = false;
  
  if (keyRepeatDelay <= 0) {
    switch (playerState) {
      case PLAYER_STATE_IDLE:
        if (playerCommons_updateZtats()) { acted = true; } else
        if (playerCommons_updateWait()) { acted = true; } else
        if (playerCommons_updateReady()) { acted = true; } else
        if (playerTown_updateCast()) { acted = true; } else
        if (playerTown_updateGet()) { acted = true; } else
        if (playerTown_updateInfo()) { acted = true; } else
        if (playerTown_updateSave()) { acted = true; } else
        if (merchantTown_updateTransact()) { acted = true; } else
        if (playerTown_updateDrop()) { acted = true; } else
        if (playerTown_updateAttack()) { acted = true; } else
        if (playerTown_updateMovement(deltaTime)) { acted = true; }
        break;
      case PLAYER_STATE_READY_TYPE:
        if (playerCommons_updateReady()) { acted = true; }
        break;
      case PLAYER_STATE_TRANSACT:
        if (merchantTown_updateTransact()) { acted = true; }
        break;
      case PLAYER_STATE_DROP:
        if (playerTown_updateDrop()) { acted = true; }
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

void playerTown_render(float *viewMatrix) {
  matrix4_setIdentity(transformMatrix);
  matrix4_setPosition(transformMatrix, player.px * OS_TOWN_CASTLE_SPRITE_WIDTH, player.py * OS_TOWN_CASTLE_SPRITE_HEIGHT, 1);

  geometry_render(&playerTownGeometry, ultimaAssets.townCastleSprites.textureId, transformMatrix, viewMatrix);
}

void playerTown_free() {
  geometry_free(&playerTownGeometry);
}