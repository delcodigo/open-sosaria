#include "sceneCastle.h"
#include "sceneDiskLoader.h"
#include "data/player.h"
#include "engine/geometry.h"
#include "engine/camera.h"
#include "entities/ui/uiConsole.h"
#include "entities/vmExecuter.h"
#include "entities/playerCastle.h"
#include "maths/matrix4.h"
#include "config.h"

static Geometry backgroundGeometry;
static float backgroundTransformationMatrix[16];

static void sceneCastle_init() {
  geometry_setSprite(&backgroundGeometry, OS_SCREEN_WIDTH, OS_SCREEN_HEIGHT, 0, 0, 1, 1);
  matrix4_setIdentity(backgroundTransformationMatrix);

  camera_setPosition3f(&camera, 0.0f, 0.0f, 10.0f);

  isPlayerInCastle = true;

  player.px = 1;
  player.py = 11;
  playerCastle_init();
}

static void sceneCastle_update(float deltaTime) {
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

    if (player_isAlive() && playerCastle_update(deltaTime)) {
      playerActed = true;
    }
  }
  
  float *viewMatrix = camera_getViewProjectionMatrix(&camera);
  geometry_render(&backgroundGeometry, ultimaAssets.castleScreen.textureId, backgroundTransformationMatrix, viewMatrix);
  playerCastle_render(viewMatrix);
  uiConsole_update(deltaTime);
}

static void sceneCastle_free() {
  geometry_free(&backgroundGeometry);
  playerCastle_free();
}

Scene sceneCastle = {
  .scene_init = sceneCastle_init,
  .scene_update = sceneCastle_update,
  .scene_free = sceneCastle_free
};