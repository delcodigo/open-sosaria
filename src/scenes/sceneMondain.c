#include <stdbool.h>
#include <math.h>
#include "sceneMondain.h"
#include "entities/ui/uiConsole.h"
#include "entities/playerMondain.h"
#include "entities/vmExecuter.h"
#include "entities/ui/uiztats.h"
#include "entities/mondain.h"
#include "entities/lightningBoltEffect.h"
#include "sceneDiskLoader.h"
#include "data/player.h"
#include "maths/vector2.h"
#include "maths/matrix4.h"
#include "engine/geometry.h"
#include "engine/texture.h"
#include "utils.h"

static Geometry gemGeometry;
static Vector2 gemPosition = {0};
static float transformMatrix[16];

int mondainMap[19][11] = {0};

bool sceneMondain_isValidPosition(int x, int y) {
  if (mondainMap[x][y] != 0) {
    return false;
  }

  if (mondain.px == x && mondain.py == y) {
    return false;
  }

  if (gemPosition.x == x && gemPosition.y == y) {
    return false;
  }

  if (player.px == x && player.py == y) {
    return false;
  }

  return true;
}

float sceneMondain_getDistanceToGem() {
  int dx = player.px - gemPosition.x;
  int dy = player.py - gemPosition.y;
  return sqrtf(dx * dx + dy * dy);
}

void sceneMondain_checkForGemTransform() {
  if (sceneMondain_getDistanceToGem() < 1.5f && sceneMondain_isGemActive()) {
    geometry_free(&gemGeometry);
    float tx1 = 80.0f / (float) ultimaAssets.mondainSprites.width;
    float tx2 = 96.0f / (float) ultimaAssets.mondainSprites.width;
    geometry_setSprite(&gemGeometry, 16, 16, tx1, 0, tx2, 1);
    if (mondain.state == MONDAIN_STATE_IDLE) {
      mondain.state = MONDAIN_STATE_ACTIVE;
    }
  }
}

bool sceneMondain_isGemActive() {
  return gemPosition.x >= 0;
}

void sceneMondain_destroyGem() {
  gemPosition.x = -15;
}

static void sceneMondain_init() {
  uiConsole_queueMessageFormat("^T1%s", ultimaStrings[1255]);
  uiConsole_queueMessageFormat("^T1%s", ultimaStrings[1256]);
  uiConsole_queueMessageFormat("^T1%s", ultimaStrings[1257]);
  uiConsole_queueMessageFormat("^T1%s", "    ");
  uiConsole_queueMessageFormat("^T1%s", ultimaStrings[1258]);
  uiConsole_queueMessage(ultimaStrings[98]);

  player.px = 5;
  player.py = 5;

  gemPosition.x = 14;
  gemPosition.y = 6;

  lightningBoltEffect_init();

  for (int i=0;i<=18;i++) {
    mondainMap[i][0] = 1;
    mondainMap[i][9] = 1;
  }

  for (int i=0;i<=10;i++) {
    mondainMap[0][i] = 1;
    mondainMap[17][i] = 1;
  }

  matrix4_setIdentity(transformMatrix);
  camera_setPosition3f(&camera, 0.0f, 0.0f, 10.0f);

  float tx1 = 32.0f / (float) ultimaAssets.mondainSprites.width;
  float tx2 = 48.0f / (float) ultimaAssets.mondainSprites.width;
  geometry_setSprite(&gemGeometry, 16, 16, tx1, 0, tx2, 1);

  mondain_init();
  playerMondain_init();
  playerActed = false;
}

static void sceneMondain_handlePlayerDeath() {
  uiConsole_queueMessageFormat("^T1%s%s", player.name, ultimaStrings[1238]);
  uiConsole_queueMessageFormat("^T1%s", ultimaStrings[1239]);
  uiConsole_queueMessageFormat("^T1%s", ultimaStrings[1240]);
  uiConsole_queueMessageFormat("^T1%s", ultimaStrings[1241]);
}

static void sceneMondain_update(float deltaTime) {
  if (lagTime > 0) {
    lagTime -= deltaTime;
    if (lagTime < 0) { lagTime = 0; }
  }
  
  if (!queuedMessagesCount && lagTime <= 0 && !vmExecuter_update(deltaTime)) {
    if (ztatsActive){
      uiZtats_update(deltaTime);
      return;
    }

    if (playerActed) {
      playerActed = false;
      mondain_update();
      if (player_isAlive()) {
        uiConsole_addMessage(ultimaStrings[98]);
      } else {
        sceneMondain_handlePlayerDeath();
      }
    }

    if (player_isAlive() && playerMondain_update(deltaTime)) {
      playerActed = true;
    }
  }
  
  float *viewMatrix = camera_getViewProjectionMatrix(&camera);
  
  lightningBoltEffect_update(deltaTime);
  lightningBoltEffect_render(viewMatrix);
  mondain_render(viewMatrix);

  matrix4_setPosition(transformMatrix, gemPosition.x * 15.0f, gemPosition.y * 15.0f, 1);
  geometry_render(&gemGeometry, ultimaAssets.mondainSprites.textureId, transformMatrix, viewMatrix);

  playerMondain_render(viewMatrix);

  uiConsole_update(deltaTime);
}

static void sceneMondain_free() {
  geometry_free(&gemGeometry);
  mondain_free();
  lightningBoltEffect_free();
  playerMondain_free();
}

Scene sceneMondain = {
  .scene_init = sceneMondain_init,
  .scene_update = sceneMondain_update,
  .scene_free = sceneMondain_free
};