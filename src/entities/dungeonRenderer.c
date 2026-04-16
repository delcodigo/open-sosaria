#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "dungeonRenderer.h"
#include "scenes/sceneDungeon.h"
#include "config.h"
#include "engine/geometry.h"
#include "engine/texture.h"
#include "maths/matrix4.h"
#include "data/bevery.h"
#include "data/player.h"

static uint8_t dungeonScreen[OS_SCREEN_WIDTH * OS_SCREEN_HEIGHT * 4] = {0};
static GLuint dungeonTextureID;
static Geometry dungeonGeometry;
static bool dungeonTextureIsLoaded = false;
static bool dungeonTextureNeedsUpdate = false;
static float dungeonTransformMatrix[16] = {0};

void dungeonRenderer_init() {
  geometry_setSprite(&dungeonGeometry, OS_SCREEN_WIDTH, OS_SCREEN_HEIGHT, 0, 0, 1, 1);
  matrix4_setIdentity(dungeonTransformMatrix);
}

void dungeonRenderer_clear() {
  memset(dungeonScreen, 0, sizeof(dungeonScreen));
  dungeonTextureNeedsUpdate = true;
}

void dungeonRenderer_setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
  if (x < 0 || x >= OS_SCREEN_WIDTH || y < 0 || y >= OS_SCREEN_HEIGHT) {
    return;
  }

  int index = (y * OS_SCREEN_WIDTH + x) * 4;
  dungeonScreen[index] = r;
  dungeonScreen[index + 1] = g;
  dungeonScreen[index + 2] = b;
  dungeonScreen[index + 3] = 255;

  dungeonTextureNeedsUpdate = true;
}

void dungeonRenderer_drawLine(int x0, int y0, int x1, int y1) {
  int dx = x1 - x0;
  int dy = y1 - y0;
  int stepX = dx > 0 ? 1 : (dx < 0 ? -1 : 0);
  int stepY = dy > 0 ? 1 : (dy < 0 ? -1 : 0);
  int absDx = abs(dx);
  int absDy = abs(dy);

  dungeonRenderer_setPixel(x0, y0, 255, 255, 255);

  if (absDx >= absDy) {
    int error = absDx;
    int previousError = error;
    int doubledDx = absDx * 2;
    int doubledDy = absDy * 2;

    for (int i = 0; i < absDx; i++) {
      x0 += stepX;
      error += doubledDy;

      if (error > doubledDx) {
        y0 += stepY;
        error -= doubledDx;

        if (error + previousError < doubledDx) {
          dungeonRenderer_setPixel(x0, y0 - stepY, 255, 255, 255);
        } else if (error + previousError > doubledDx) {
          dungeonRenderer_setPixel(x0 - stepX, y0, 255, 255, 255);
        } else {
          dungeonRenderer_setPixel(x0, y0 - stepY, 255, 255, 255);
          dungeonRenderer_setPixel(x0 - stepX, y0, 255, 255, 255);
        }
      }

      dungeonRenderer_setPixel(x0, y0, 255, 255, 255);
      previousError = error;
    }
  } else {
    int error = absDy;
    int previousError = error;
    int doubledDx = absDx * 2;
    int doubledDy = absDy * 2;

    for (int i = 0; i < absDy; i++) {
      y0 += stepY;
      error += doubledDx;

      if (error > doubledDy) {
        x0 += stepX;
        error -= doubledDy;

        if (error + previousError < doubledDy) {
          dungeonRenderer_setPixel(x0 - stepX, y0, 255, 255, 255);
        } else if (error + previousError > doubledDy) {
          dungeonRenderer_setPixel(x0, y0 - stepY, 255, 255, 255);
        } else {
          dungeonRenderer_setPixel(x0 - stepX, y0, 255, 255, 255);
          dungeonRenderer_setPixel(x0, y0 - stepY, 255, 255, 255);
        }
      }

      dungeonRenderer_setPixel(x0, y0, 255, 255, 255);
      previousError = error;
    }
  }
}

static int dungeonRenderer_getLeftTile(int distance) {
  return dungeonMap[player.px + player.dx * distance + player.dy][player.py + player.dy * distance - player.dx] % 100;
}

static int dungeonRenderer_getRightTile(int distance) {
  return dungeonMap[player.px + player.dx * distance - player.dy][player.py + player.dy * distance + player.dx] % 100;
}

static int dungeonRenderer_getFrontTile(int distance) {
  return dungeonMap[player.px + player.dx * distance][player.py + player.dy * distance] % 100;
}

void dungeonRenderer_update() {
  dungeonRenderer_clear();
  bool shouldBreak = false;

  for (int distance=0;distance<OS_DUNGEON_TABLE_HEIGHT;distance++) {
    int centerTile = dungeonRenderer_getFrontTile(distance);
    int leftTile = dungeonRenderer_getLeftTile(distance);
    int rightTile = dungeonRenderer_getRightTile(distance);

    int l1 = dungeonTable[distance][0];
    int r1 = dungeonTable[distance][1];
    int t1 = dungeonTable[distance][2];
    int b1 = dungeonTable[distance][3];

    int l2 = dungeonTable[distance + 1][0];
    int r2 = dungeonTable[distance + 1][1];
    int t2 = dungeonTable[distance + 1][2];
    int b2 = dungeonTable[distance + 1][3];

    if (distance > 0) {
      if (centerTile == 1 || centerTile == 3 || centerTile == 4) {
        dungeonRenderer_drawLine(l1, t1, r1, t1);
        dungeonRenderer_drawLine(r1, b1, l1, b1);

        int previousAdjacentTile = dungeonRenderer_getLeftTile(distance - 1);
        if (previousAdjacentTile == 1 || previousAdjacentTile == 3 || previousAdjacentTile == 4) {
          dungeonRenderer_drawLine(l1, t1, l1, b1);
        }

        previousAdjacentTile = dungeonRenderer_getRightTile(distance - 1);
        if (previousAdjacentTile == 1 || previousAdjacentTile == 3 || previousAdjacentTile == 4) {
          dungeonRenderer_drawLine(r1, t1, r1, b1);
        }
      }

      if (centerTile == 1 || centerTile == 3) {
        shouldBreak = true;

      }

      if (centerTile == 4) {
        shouldBreak = true;

      }
    }

    if (!shouldBreak) {
      if (centerTile == 6) {

      }
      if (centerTile == 12) {

      }
      if (leftTile == 1 || leftTile == 3 || leftTile == 4) {
        dungeonRenderer_drawLine(l1, t1, l2, t2);
        dungeonRenderer_drawLine(l1, b1, l2, b2);
      } else {
        if (distance != 0) {
          dungeonRenderer_drawLine(l1, t1, l1, b1);
        }

        dungeonRenderer_drawLine(l1, t2, l2, t2);
        dungeonRenderer_drawLine(l2, b2, l1, b2);

        int frontTile = dungeonRenderer_getFrontTile(distance + 1);
        if (frontTile != 1 && frontTile != 3 && frontTile != 4) {
          dungeonRenderer_drawLine(l2, t2, l2, b2);
        }
      }
      if (rightTile == 1 || rightTile == 3 || rightTile == 4) {
        dungeonRenderer_drawLine(r1, t1, r2, t2);
        dungeonRenderer_drawLine(r1, b1, r2, b2);
      } else {
        if (distance != 0) {
          dungeonRenderer_drawLine(r1, t1, r1, b1);
        }

        dungeonRenderer_drawLine(r1, t2, r2, t2);
        dungeonRenderer_drawLine(r2, b2, r1, b2);

        int frontTile = dungeonRenderer_getFrontTile(distance + 1);
        if (frontTile != 1 && frontTile != 3 && frontTile != 4) {
          dungeonRenderer_drawLine(r2, t2, r2, b2);
        }
      }
      if (centerTile == 7 || centerTile == 9) {

      }
      if (centerTile == 8) {

      }
      if (centerTile == 7 || centerTile == 8) {
        if (player.dy == 0) {

        } else {

        }
      }
      if (centerTile == 5) {

      }
    }
    if ((int)(dungeonMap[player.px][player.py] / 100) < 1) {
      if (shouldBreak) {
        break;
      }
    } else {

    }
  }

  dungeonTextureNeedsUpdate = true;
}

void dungeonRenderer_render(float *viewMatrix) {
  if (dungeonTextureNeedsUpdate || !dungeonTextureIsLoaded) {
    if (dungeonTextureIsLoaded) {
      texture_free(dungeonTextureID);
    }

    dungeonTextureIsLoaded = true;
    dungeonTextureID = texture_load(OS_SCREEN_WIDTH, OS_SCREEN_HEIGHT, dungeonScreen);
    dungeonTextureNeedsUpdate = false;
  }

  geometry_render(&dungeonGeometry, dungeonTextureID, dungeonTransformMatrix, viewMatrix);
}

void dungeonRenderer_free() {
  if (dungeonTextureIsLoaded) {
    texture_free(dungeonTextureID);
    dungeonTextureIsLoaded = false;
  }

  geometry_free(&dungeonGeometry);
}