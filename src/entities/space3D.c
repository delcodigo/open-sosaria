#include <stdbool.h>
#include "space3D.h"
#include "engine/geometry.h"
#include "engine/texture.h"
#include "maths/matrix4.h"
#include "config.h"

static Geometry screenGeometry;
static unsigned char screenData[OS_SCREEN_WIDTH * OS_SCREEN_HEIGHT * 4] = {0};
static GLuint screenTexture;
static bool textureNeedsUpdate;
static float transformMatrix[16];

static void space_setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
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
  for (int y=0;y<160;y++) {
    int column = 0;
    if (y < 129) { column = 257; }

    for (int x=column;x<OS_SCREEN_WIDTH;x++) {
      space_setPixel(x, y, 36, 182, 0);
    }
  }

  for (int x=0;x<257;x++) {
    space_setPixel(x, 0, 255, 255, 255);
    space_setPixel(x, 129, 255, 255, 255);
  }

  for (int y=0;y<129;y++) {
    space_setPixel(0, y, 255, 255, 255);
    space_setPixel(256, y, 255, 255, 255);
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

void space3D_free() {
  texture_free(screenTexture);
  geometry_free(&screenGeometry);
}