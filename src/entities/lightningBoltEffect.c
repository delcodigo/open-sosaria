#include <stdbool.h>
#include <stdlib.h>
#include "lightningBoltEffect.h"
#include "engine/geometry.h"
#include "engine/texture.h"
#include "maths/matrix4.h"
#include "maths/vector2.h"
#include "scenes/sceneMondain.h"
#include "config.h"
#include "utils.h"

static Geometry screenGeometry;
static unsigned char screenData[OS_SCREEN_WIDTH * OS_SCREEN_HEIGHT * 4] = {0};
static GLuint screenTexture;
static float transformMatrix[16];
static Vector2 lightningBoltPosition = {0};
static int lightningBoltStep = 0;
static float lightningBoltDelay = 0;
static bool textureNeedsUpdate = false;

static void lightningBoltEffect_initBackground() {
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

void lightningBoltEffect_init() {
	lightningBoltEffect_initBackground();
	matrix4_setIdentity(transformMatrix);

	lightningBoltStep = 0;
	lightningBoltDelay = 0;
}

void lightningBoltEffect_cast() {
	if (lightningBoltEffect_isBusy()) { return; }
	
	do {
		lightningBoltPosition.x = (int)(rand01() * 10 + 4);
		lightningBoltPosition.y = (int)(rand01() * 6 + 2);
	} while (!sceneMondain_isValidPosition(lightningBoltPosition.x, lightningBoltPosition.y));

	lightningBoltStep = 1;
}

static void lightningBoltEffect_setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
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

static void lightningBoltEffect_drawLine(int x0, int y0, int x1, int y1, int r, int g, int b) {
  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int stepX = x0 < x1 ? 1 : -1;
  int stepY = y0 < y1 ? 1 : -1;
  int err = dx - dy;

  while (1) {
    lightningBoltEffect_setPixel(x0, y0, r, g, b);
    if (x0 == x1 && y0 == y1) break;
    int e2 = 2 * err;
    if (e2 > -dy) { err -= dy; x0 += stepX; } else
    if (e2 < dx)  { err += dx; y0 += stepY; }
  }
}

bool lightningBoltEffect_isBusy() {
	return lightningBoltStep != 0;
}

void lightningBoltEffect_update(float deltaTime) {
	if (lightningBoltStep == 0) { return; }

	int xx = lightningBoltPosition.x;
	int yy = lightningBoltPosition.y;
	int r = (lightningBoltStep < 9) ? 255 : 0;
	int g = (lightningBoltStep < 9) ? 86 : 0;
	int b = 0;

  if (lightningBoltDelay <= 0) {
		switch (lightningBoltStep) {
			case 1:
			case 9:
				lightningBoltEffect_drawLine(xx*15, yy*15+7, xx*15, yy*15-7, r, g, b);
				break;
			case 2:
			case 10:
				lightningBoltEffect_drawLine(xx*15, yy*15+7, xx*15+7, yy*15, r, g, b);
				break;
			case 3:
			case 11:
				lightningBoltEffect_drawLine(xx*15, yy*15+7, xx*15-7, yy*15, r, g, b);
				break;
			case 4:
			case 12:
				lightningBoltEffect_drawLine(xx*15, yy*15+7, xx*15+7, yy*15+5, r, g, b);
				break;
			case 5:
			case 13:
				lightningBoltEffect_drawLine(xx*15, yy*15+7, xx*15-7, yy*15+5, r, g, b);
				break;
			case 6:
			case 7:
			case 14:
			case 15:
				lightningBoltEffect_drawLine(xx*15, yy*15+7, xx*15+4, yy*15-4, r, g, b);
				break;
			case 8:
			case 16:
				lightningBoltEffect_drawLine(xx*15, yy*15+7, xx*15-4, yy*15-4, r, g, b);
				break;
			case 17:
				lightningBoltStep = -1;
				break;
			default:
				break;
		}

		lightningBoltStep++;
		lightningBoltDelay = 0.01f;
	} else {
		lightningBoltDelay -= deltaTime;
	}
}

void lightningBoltEffect_render(float *viewMatrix) {
	if (textureNeedsUpdate) {
    texture_update(screenTexture, OS_SCREEN_WIDTH, OS_SCREEN_HEIGHT, screenData);
    textureNeedsUpdate = false;
  }

	geometry_render(&screenGeometry, screenTexture, transformMatrix, viewMatrix);
}

void lightningBoltEffect_free() {
	texture_free(screenTexture);
  geometry_free(&screenGeometry);
}