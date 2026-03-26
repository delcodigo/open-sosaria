#include "playerTown.h"
#include "engine/geometry.h"
#include "engine/input.h"
#include "entities/ui/uiConsole.h"
#include "scenes/sceneDiskLoader.h"
#include "data/player.h"
#include "maths/matrix4.h"
#include "config.h"

static Geometry playerTownGeometry;
static float transformMatrix[16];

void playerTown_init() {
  float tx1 = (6.0f * OS_TOWN_CASTLE_SPRITE_WIDTH) / (float)ultimaAssets.townCastleSprites.width;
  float tx2 = (7.0f * OS_TOWN_CASTLE_SPRITE_WIDTH) / (float)ultimaAssets.townCastleSprites.width;

  geometry_setSprite(&playerTownGeometry, OS_TOWN_CASTLE_SPRITE_WIDTH, OS_TOWN_CASTLE_SPRITE_HEIGHT, tx1, 0, tx2, 1);
}

static bool playerTown_updateMovement(float deltaTime) {
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

    if (ultimaAssets.townCollisionMap[player.py + moveY][player.px + moveX]) {
      uiConsole_addMessage(ultimaStrings[341]);
      keyRepeatDelay = 0.3f;
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

bool playerTown_update(float deltaTime) {
  bool acted = false;
  
  if (keyRepeatDelay <= 0) {
    if (playerTown_updateMovement(deltaTime)) { acted = true; }
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