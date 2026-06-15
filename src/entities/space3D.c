#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include "space3D.h"
#include "engine/geometry.h"
#include "engine/texture.h"
#include "maths/matrix4.h"
#include "maths/vector2.h"
#include "entities/playerSpace.h"
#include "entities/playerCommons.h"
#include "entities/ui/uiConsole.h"
#include "data/player.h"
#include "scenes/sceneDiskLoader.h"
#include "scenes/sceneSpace.h"
#include "config.h"
#include "utils.h"

static Geometry screenGeometry;
static Geometry enemyCraftGeometry;
static unsigned char screenData[OS_SCREEN_WIDTH * OS_SCREEN_HEIGHT * 4] = {0};
static GLuint screenTexture;
static bool textureNeedsUpdate;
static float transformMatrix[16];
static Vector2f starsPosition[14] = {0};
static Vector2f enemyCraftPosition = {0};
static int enemyCraftFacing = 0;
static float enemyCraftTransformMatrix[16] = {0};
static float enemyCraftInAttackPosition = 0;
static float enemyCraftAttacked = 0;
static const int starsCount = sizeof(starsPosition) / sizeof(Vector2f);
static float starsSpreadRate = 1;
static uint8_t colorPurple[3] = {146, 0, 255};
static uint8_t colorGreen[3] = {36, 182, 0};
static float hyperJumpKeepTimer = 0;
float starsSpeedModifier = 1;
HYPER_JUMP_STATE hyperJumpingState = HYPER_JUMP_STATE_OFF;

static void space3D_initStars() {
  targetCentre.x = 128;
  targetCentre.y = 64;

  for (int i=0;i<starsCount;i++) {
    starsPosition[i].x = rand01() * 255;
    starsPosition[i].y = rand01() * 127;
  }
}

static void space3D_setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
  if (x < 0 || x >= OS_SCREEN_WIDTH || y < 0 || y >= OS_SCREEN_HEIGHT) {
    return;
  }

  int index = (y * OS_SCREEN_WIDTH + x) * 4;
  screenData[index] = r;
  screenData[index + 1] = g;
  screenData[index + 2] = b;
  screenData[index + 3] = 255;

  textureNeedsUpdate = true;
}

static void space3D_drawLine(int x0, int y0, int x1, int y1, int r, int g, int b) {
  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int stepX = x0 < x1 ? 1 : -1;
  int stepY = y0 < y1 ? 1 : -1;
  int err = dx - dy;

  while (1) {
    space3D_setPixel(x0, y0, r, g, b);
    if (x0 == x1 && y0 == y1) break;
    int e2 = 2 * err;
    if (e2 > -dy) { err -= dy; x0 += stepX; } else
    if (e2 < dx)  { err += dx; y0 += stepY; }
  }
}

void space3D_init() {
  space3D_initStars();

  float tx1 = 0.0f;
  float ty1 = 24.0f / (float)ultimaAssets.spaceSprites.height;
  float tx2 = 24.0f / (float)ultimaAssets.spaceSprites.width;
  float ty2 = 48.0f / (float)ultimaAssets.spaceSprites.height;
  geometry_setSpriteOffset(&enemyCraftGeometry, 12, 12, 24, 24, tx1, ty1, tx2, ty2);

  for (int y=0;y<160;y++) {
    int column = 0;
    if (y < 129) { column = 257; }

    for (int x=column;x<OS_SCREEN_WIDTH;x++) {
      space3D_setPixel(x, y, 36, 182, 0);
    }
  }

  for (int x=0;x<257;x++) {
    space3D_setPixel(x, 0, 255, 255, 255);
    space3D_setPixel(x, 129, 255, 255, 255);
  }

  for (int y=0;y<129;y++) {
    space3D_setPixel(0, y, 255, 255, 255);
    space3D_setPixel(256, y, 255, 255, 255);
  }

  screenTexture = texture_load(OS_SCREEN_WIDTH, OS_SCREEN_HEIGHT, screenData);
  geometry_setSprite(&screenGeometry, OS_SCREEN_WIDTH, OS_SCREEN_HEIGHT, 0, 0, 1, 1);

  matrix4_setIdentity(transformMatrix);
}

static void space3D_clearEnemyAttackLines(float deltaTime) {
  enemyCraftAttacked -= deltaTime;

  if (enemyCraftAttacked <= 0) {
    enemyCraftAttacked = 0;
    space3D_drawLine(enemyCraftPosition.x + 8, enemyCraftPosition.y, enemyCraftPosition.x, 127, 0, 0, 0);
    space3D_drawLine(enemyCraftPosition.x, 127, enemyCraftPosition.x - 8, enemyCraftPosition.y + 8, 0, 0, 0);
  }
}

void space3D_render(float *viewMatrix) {
  if (textureNeedsUpdate) {
    texture_update(screenTexture, OS_SCREEN_WIDTH, OS_SCREEN_HEIGHT, screenData);
    textureNeedsUpdate = false;
  }

  geometry_render(&screenGeometry, screenTexture, transformMatrix, viewMatrix);

  if (enemyCrafts > 0) {
    if (enemyCraftFacing > 128) {
      matrix4_setIdentity(enemyCraftTransformMatrix);
      matrix4_setPosition(enemyCraftTransformMatrix, (int)enemyCraftPosition.x, (int)enemyCraftPosition.y, 2);
      geometry_render(&enemyCraftGeometry, ultimaAssets.spaceSprites.textureId, enemyCraftTransformMatrix, viewMatrix);
    }
  }
}

static void space3D_updateHyperJump(float deltaTime) {
  switch (hyperJumpingState) {
    case HYPER_JUMP_STATE_ACCELERATE:
      starsSpeedModifier += deltaTime * 1;
      if (starsSpeedModifier >= 9){
        starsSpeedModifier = 9;
        hyperJumpKeepTimer = 5;
        hyperJumpingState = HYPER_JUMP_STATE_KEEP;
      }
      break;

    case HYPER_JUMP_STATE_KEEP:
      hyperJumpKeepTimer -= deltaTime;
      if (hyperJumpKeepTimer <= 0) {
        hyperJumpKeepTimer = 0;
        hyperJumpingState = HYPER_JUMP_STATE_STOP;
      }
      break;

    case HYPER_JUMP_STATE_STOP:
      starsSpeedModifier -= deltaTime * 2;
      if (starsSpeedModifier <= 1){
        starsSpeedModifier = 1;
        hyperJumpingState = HYPER_JUMP_STATE_OFF;
        playerState = PLAYER_STATE_SPACE_FIRST_PERSON;
        playerSpace_performHyperJump();
        sceneSpace_initializeShapes();
        uiConsole_replaceLastMessage(ultimaStrings[98]);
      }
      break;

    default:
      break;
  }
}

static void space3D_updateEnemyCraft(float deltaTime) {
  if (enemyCrafts <= 0) { return; }

  float step = (11.0f - starsSpeedModifier) * deltaTime;
  
  if (targetCentre.x == 176) { enemyCraftPosition.x -= step; } else
  if (targetCentre.x == 80) { enemyCraftPosition.x += step; } else 
  if (targetCentre.x < enemyCraftPosition.x) { enemyCraftPosition.x += step; } else 
  if (targetCentre.x > enemyCraftPosition.x) { enemyCraftPosition.x -= step; }
  
  if (targetCentre.y == 88) { enemyCraftPosition.y -= step; } else
  if (targetCentre.y == 40) { enemyCraftPosition.y += step; } else 
  if (targetCentre.y < enemyCraftPosition.y) { enemyCraftPosition.y += step; } else 
  if (targetCentre.y > enemyCraftPosition.y) { enemyCraftPosition.y -= step; }

  if (enemyCraftPosition.x < 9 || enemyCraftPosition.x >= 245 || enemyCraftPosition.y < 9 || enemyCraftPosition.y >= 118) {
    enemyCraftFacing = (enemyCraftFacing < 128) ? 96 : 0;
  } else {
    enemyCraftFacing = (enemyCraftFacing == 255) ? 255 : 192;

    if (enemyCraftPosition.x >= 112 && enemyCraftPosition.x < 143 && enemyCraftPosition.y >= 48 && enemyCraftPosition.y < 79) {
      enemyCraftInAttackPosition += deltaTime;
      if (enemyCraftInAttackPosition >= 2) {
        enemyCraftInAttackPosition = 0;
        enemyCraftAttacked = 0.5f;

        space3D_drawLine(enemyCraftPosition.x + 8, enemyCraftPosition.y, enemyCraftPosition.x, 127, 255, 86, 0);
        space3D_drawLine(enemyCraftPosition.x, 127, enemyCraftPosition.x - 8, enemyCraftPosition.y + 8, 255, 86, 0);

        uiConsole_queueMessage(ultimaStrings[1011]);
        uiConsole_queueMessage(ultimaStrings[98]);

        player.shield -= 321;
        if (player.shield <= 0) { 
          player.shield = 0; 
        }

        uiConsole_updateSpaceStats();
      }
    }

  }

  enemyCraftPosition.x = fmodf(enemyCraftPosition.x + 255.0f, 255.0f);
  enemyCraftPosition.y = fmodf(enemyCraftPosition.y + 128.0f, 128.0f);
}

void space3D_update(float deltaTime) {
  if (enemyCraftAttacked > 0) {
    space3D_clearEnemyAttackLines(deltaTime);
    return;
  }
  
  space3D_updateHyperJump(deltaTime);

  for (int i=0;i<starsCount;i++) {
    float newX = starsPosition[i].x + (starsPosition[i].x - (float)targetCentre.x) * starsSpreadRate * deltaTime * starsSpeedModifier;
    float newY = starsPosition[i].y + (starsPosition[i].y - (float)targetCentre.y) * starsSpreadRate * deltaTime * starsSpeedModifier;

    if (newX < 1 || newY < 1 || newX > 255 || newY > 127) {
      newX = (float)targetCentre.x + rand01() * 32 - 16;
      newY = (float)targetCentre.y + rand01() * 16 - 8;
    }

    space3D_setPixel((int)starsPosition[i].x, (int)starsPosition[i].y, 0, 0, 0);

    starsPosition[i].x = newX;
    starsPosition[i].y = newY;

    int r = colorPurple[0];
    int g = colorPurple[1];
    int b = colorPurple[2];

    if ((((int)newX) & 1) == 0) {
      r = colorGreen[0];
      g = colorGreen[1];
      b = colorGreen[2];
    }

    space3D_setPixel((int)starsPosition[i].x, (int)starsPosition[i].y, r, g, b);
  }

  space3D_updateEnemyCraft(deltaTime);
}

void space3D_free() {
  texture_free(screenTexture);
  geometry_free(&screenGeometry);
}