#include <stdbool.h>
#include "sceneMondain.h"
#include "entities/ui/uiConsole.h"
#include "entities/playerMondain.h"
#include "entities/vmExecuter.h"
#include "entities/ui/uiztats.h"
#include "sceneDiskLoader.h"
#include "data/player.h"
#include "maths/vector2.h"
#include "maths/matrix4.h"
#include "engine/geometry.h"
#include "engine/texture.h"

static Geometry mondainGeometry;
static Vector2 mondainPosition = {0};
static Geometry gemGeometry;
static Vector2 gemPosition = {0};
static Geometry screenGeometry;
static unsigned char screenData[OS_SCREEN_WIDTH * OS_SCREEN_HEIGHT * 4] = {0};
static GLuint screenTexture;
static float transformMatrix[16];

int mondainMap[19][11] = {0};

static void sceneMondain_initBackground() {
  for (int i=0;i<=6;i++) {
    for (int x=i;x<=272-i;x++) {
      int index = (i * OS_SCREEN_WIDTH + x) * 4;
      screenData[index] = 0;
      screenData[index + 1] = 146;
      screenData[index + 2] = 255;
      screenData[index + 3] = 255;

      index = ((150 - i) * OS_SCREEN_WIDTH + x) * 4;
      screenData[index] = 0;
      screenData[index + 1] = 146;
      screenData[index + 2] = 255;
      screenData[index + 3] = 255;
    }

    for (int y=i;y<=150-i;y++) {
      int index = (y * OS_SCREEN_WIDTH + i) * 4;
      screenData[index] = 0;
      screenData[index + 1] = 146;
      screenData[index + 2] = 255;
      screenData[index + 3] = 255;

      index = (y * OS_SCREEN_WIDTH + (272-i)) * 4;
      screenData[index] = 0;
      screenData[index + 1] = 146;
      screenData[index + 2] = 255;
      screenData[index + 3] = 255;
    }
  }

  geometry_setSprite(&screenGeometry, OS_SCREEN_WIDTH, OS_SCREEN_HEIGHT, 0, 0, 1, 1);
  screenTexture = texture_load(OS_SCREEN_WIDTH, OS_SCREEN_HEIGHT, screenData);
}

bool sceneMondain_isValidPosition(int x, int y) {
  if (mondainMap[x][y] != 0) {
    return false;
  }

  if (mondainPosition.x == x && mondainPosition.y == y) {
    return false;
  }

  if (gemPosition.x == x && gemPosition.y == y) {
    return false;
  }

  return true;
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

  mondainPosition.x = 15;
  mondainPosition.y = 6;

  gemPosition.x = 14;
  gemPosition.y = 6;

  sceneMondain_initBackground();

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

  float tx1 = 16.0f / (float) ultimaAssets.mondainSprites.width;
  float tx2 = 32.0f / (float) ultimaAssets.mondainSprites.width;
  geometry_setSprite(&mondainGeometry, 16, 16, tx1, 0, tx2, 1);

  tx1 = 32.0f / (float) ultimaAssets.mondainSprites.width;
  tx2 = 48.0f / (float) ultimaAssets.mondainSprites.width;
  geometry_setSprite(&gemGeometry, 16, 16, tx1, 0, tx2, 1);

  playerMondain_init();
  playerActed = false;
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
      if (player_isAlive()) {
        uiConsole_addMessage(ultimaStrings[98]);
      }
    }

    if (player_isAlive() && playerMondain_update(deltaTime)) {
      playerActed = true;
    }
  }
  
  float *viewMatrix = camera_getViewProjectionMatrix(&camera);
  matrix4_setIdentity(transformMatrix);
  geometry_render(&screenGeometry, screenTexture, transformMatrix, viewMatrix);

  matrix4_setPosition(transformMatrix, mondainPosition.x * 15.0f, mondainPosition.y * 15.0f, 1);
  geometry_render(&mondainGeometry, ultimaAssets.mondainSprites.textureId, transformMatrix, viewMatrix);

  matrix4_setPosition(transformMatrix, gemPosition.x * 15.0f, gemPosition.y * 15.0f, 1);
  geometry_render(&gemGeometry, ultimaAssets.mondainSprites.textureId, transformMatrix, viewMatrix);

  playerMondain_render(viewMatrix);

  uiConsole_update(deltaTime);
}

static void sceneMondain_free() {
  texture_free(screenTexture);
  geometry_free(&mondainGeometry);
  geometry_free(&gemGeometry);
  playerMondain_free();
}

Scene sceneMondain = {
  .scene_init = sceneMondain_init,
  .scene_update = sceneMondain_update,
  .scene_free = sceneMondain_free
};