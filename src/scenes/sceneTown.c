#include "sceneTown.h"
#include "sceneDiskLoader.h"
#include "data/player.h"
#include "engine/geometry.h"
#include "maths/matrix4.h"
#include "entities/ui/uiConsole.h"
#include "entities/playerTown.h"
#include "entities/vmExecuter.h"
#include "entities/guardTown.h"
#include "config.h"

static Geometry townGeometry;
static float townTransform[16];

static void sceneTown_init() {
  geometry_setSprite(&townGeometry, OS_SCREEN_WIDTH, OS_SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f);
  matrix4_setIdentity(townTransform);

  camera_setPosition3f(&camera, 0.0f, 0.0f, 10.0f);

  player.px = 20;
  player.py = 20;
  playerTown_init();
  guardTown_init();
}

bool sceneTown_isSolid(int x, int y) {
  if (ultimaAssets.townCollisionMap[y][x]) {
    return true;
  }

  for (int i=0; i<OS_GUARD_TOWN_COUNT; i++) {
    if (guardTowns[i].x == x && guardTowns[i].y == y) {
      return true;
    }
  }

  return false;
}

static void sceneTown_update(float deltaTime) {
  if (lagTime > 0) {
    lagTime -= deltaTime;
    if (lagTime < 0) { lagTime = 0; }
  }
  
  if (!queuedMessagesCount && lagTime <= 0 && !vmExecuter_update(deltaTime)) {
    if (playerActed) {
      playerActed = false;
      
      if (player_isAlive()) {
        uiConsole_addMessage(ultimaStrings[98]);
      }
    }

    if (player_isAlive() && playerTown_update(deltaTime)) {
      playerActed = true;
    }
  }

  float *viewMatrix = camera_getViewProjectionMatrix(&camera);
  geometry_render(&townGeometry, ultimaAssets.townScreen.textureId, townTransform, viewMatrix);

  playerTown_render(viewMatrix);
  guardTown_render(viewMatrix);
  uiConsole_update(deltaTime);
}

static void sceneTown_free() {
  geometry_free(&townGeometry);
  playerTown_free();
  guardTown_free();
}

Scene sceneTown = {
  .scene_init = sceneTown_init,
  .scene_update = sceneTown_update,
  .scene_free = sceneTown_free
};