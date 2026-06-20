#include "playerMondain.h"
#include "engine/geometry.h"
#include "engine/texture.h"
#include "data/player.h"
#include "scenes/sceneDiskLoader.h"
#include "scenes/sceneMondain.h"
#include "maths/matrix4.h"
#include "engine/input.h"
#include "entities/ui/uiConsole.h"

static Geometry playerGeometry;
static float transformMatrix[16];

void playerMondain_init() {
  float tx1 = 0;
  float tx2 = 16.0f / (float) ultimaAssets.mondainSprites.width;
  geometry_setSprite(&playerGeometry, 16, 16, tx1, 0, tx2, 1);

  matrix4_setIdentity(transformMatrix);
}

void playerMondain_render(float *viewMatrix) {
  matrix4_setPosition(transformMatrix, player.px * 15, player.py * 15, 1);
  geometry_render(&playerGeometry, ultimaAssets.mondainSprites.textureId, transformMatrix, viewMatrix);
}

static bool playerMondain_updateMovement(float deltaTime) {
  int directionMessageIndex = 0;
  int dx = 0;
  int dy = 0;

  if (input.up) {
    directionMessageIndex = 117;
    dy = -1;
  } else if (input.down) {
    directionMessageIndex = 118;
    dy = 1;
  } else if (input.right) {
    directionMessageIndex = 119;
    dx = 1;
  } else if (input.left) {
    directionMessageIndex = 120;
    dx = -1;
  }

  if (dx != 0 || dy != 0) {
    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[directionMessageIndex]);

    if (!sceneMondain_isValidPosition(player.px + dx, player.py + dy)) {
      uiConsole_queueMessage(ultimaStrings[1126]);
      int damage = (int)((float)player.health / 10.0f);
      player.health -= damage;
      keyRepeatDelay = 0.3f;
      uiConsole_queueMessageFormat("%s%d", ultimaStrings[1127], damage);
      uiConsole_updateStats();
      return true;
    }

    player.px += dx;
    player.py += dy;

    keyRepeatDelay = 0.1f;
    player_consumeTownFood();
    return true;
  } else {
    waitingTime += deltaTime;
    if (waitingTime >= 5.0f) {
      waitingTime = 0.0f;
      uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[99]);
      player_waitPenalty();
      return true;
    }
  }

  return false;
}

bool playerMondain_update(float deltaTime) {
  bool acted = false;

  if (keyRepeatDelay <= 0.0f) {
    if (playerMondain_updateMovement(deltaTime)) { acted = true; }
  } else {
    keyRepeatDelay -= deltaTime;
    if (keyRepeatDelay < 0) {
      keyRepeatDelay = 0;
    }
  }

  return acted;
}

void playerMondain_free() {
  geometry_free(&playerGeometry);
}