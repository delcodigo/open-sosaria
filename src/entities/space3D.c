#include <stdbool.h>
#include "space3D.h"
#include "engine/geometry.h"
#include "engine/texture.h"
#include "maths/matrix4.h"
#include "maths/vector2.h"
#include "entities/playerSpace.h"
#include "config.h"
#include "utils.h"

static Geometry screenGeometry;
static unsigned char screenData[OS_SCREEN_WIDTH * OS_SCREEN_HEIGHT * 4] = {0};
static GLuint screenTexture;
static bool textureNeedsUpdate;
static float transformMatrix[16];
static Vector2f starsPosition[14] = {0};
static const int starsCount = sizeof(starsPosition) / sizeof(Vector2f);
static float starsSpreadRate = 1;
static uint8_t colorPurple[3] = {146, 0, 255};
static uint8_t colorGreen[3] = {36, 182, 0};

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

void space3D_init() {
  space3D_initStars();

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

void space3D_render(float *viewMatrix) {
  if (textureNeedsUpdate) {
    texture_update(screenTexture, OS_SCREEN_WIDTH, OS_SCREEN_HEIGHT, screenData);
    textureNeedsUpdate = false;
  }

  geometry_render(&screenGeometry, screenTexture, transformMatrix, viewMatrix);
}

void space3D_update(float delta) {
  for (int i=0;i<starsCount;i++) {
    float newX = starsPosition[i].x + (starsPosition[i].x - targetCentre.x) * starsSpreadRate * delta;
    float newY = starsPosition[i].y + (starsPosition[i].y - targetCentre.y) * starsSpreadRate * delta;

    if (newX < 1 || newY < 1 || newX > 255 || newY > 127) {
      newX = targetCentre.x + rand01() * 32 - 16;
      newY = targetCentre.y + rand01() * 16 - 8;
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
}

void space3D_free() {
  texture_free(screenTexture);
  geometry_free(&screenGeometry);
}