#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "engine/geometry.h"
#include "dungeonRenderer.h"
#include "scenes/sceneDungeon.h"
#include "scenes/sceneDiskLoader.h"
#include "config.h"
#include "engine/texture.h"
#include "maths/matrix4.h"
#include "data/bevery.h"
#include "data/player.h"
#include "data/dungeonEnemy.h"
#include "data/enemy.h"
#include "entities/ui/uiConsole.h"

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
  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int stepX = x0 < x1 ? 1 : -1;
  int stepY = y0 < y1 ? 1 : -1;
  int err = dx - dy;

  while (1) {
    dungeonRenderer_setPixel(x0, y0, 255, 255, 255);
    if (x0 == x1 && y0 == y1) break;
    int e2 = 2 * err;
    if (e2 > -dy) { err -= dy; x0 += stepX; }
    if (e2 < dx)  { err += dx; y0 += stepY; }
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

        dungeonRenderer_drawLine(dungeonDoorsFrontTable[distance][0], dungeonDoorsFrontTable[distance][3], dungeonDoorsFrontTable[distance][0], dungeonDoorsFrontTable[distance][2]);
        dungeonRenderer_drawLine(dungeonDoorsFrontTable[distance][0], dungeonDoorsFrontTable[distance][2], dungeonDoorsFrontTable[distance][1], dungeonDoorsFrontTable[distance][2]);
        dungeonRenderer_drawLine(dungeonDoorsFrontTable[distance][1], dungeonDoorsFrontTable[distance][2], dungeonDoorsFrontTable[distance][1], dungeonDoorsFrontTable[distance][3]);
      }
    }

    if (!shouldBreak) {
      if (centerTile == 6) {
        int t3 = (b2 * 3 + b1) / 4;
        int t4 = (b2 + b1) / 2;
        int t5 = (l2 * 7 + r2 * 3) / 10;
        int t6 = (l2 * 1.5f + r2 * 8.5f) / 10;
        int l3 = (l2 * 8 + r2 * 2) / 10;
        int r3 = (l2 + r2 * 9) / 10;
        int t7 = (b2 + b1 * 3) / 4;

        dungeonRenderer_drawLine(l3,t3,t5,b2);
        dungeonRenderer_drawLine(t5,b2,r3,t3);
        dungeonRenderer_drawLine(r3,t3,r3,t7);
        dungeonRenderer_drawLine(r3,t7,t6,b1);
        dungeonRenderer_drawLine(t6,b1,t5,b1);
        dungeonRenderer_drawLine(t5,b1,l3,t7);
        dungeonRenderer_drawLine(l3,t7,l3,t3);
        dungeonRenderer_drawLine(l3,t3,t5,t4);
        dungeonRenderer_drawLine(t5,t4,t6,t4);
        dungeonRenderer_drawLine(t6,t4,t6,b1);
        dungeonRenderer_drawLine(t5,b1,t5,t4);
        dungeonRenderer_drawLine(t6,t4,r3,t3);

        uiConsole_queueMessageFormat("^1%s^0", ultimaStrings[843]);
      }
      if (centerTile == 12) {
        int l = (l1 + l2) / 2;
        int r = (r1 + r2) / 2;
        int t = (t1 + t2) / 2;
        int b = (b1 + b2) / 2;
        for (int x=1;x<=4;x++) {
          int h = (t * x + (5 - x) * b) / 5;
          dungeonRenderer_drawLine(l, h, r, h);
        }
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
      if (leftTile == 4) {
        if (distance > 0) {
          dungeonRenderer_drawLine(dungeonDoorsTable[distance][0], dungeonDoorsTable[distance][4], dungeonDoorsTable[distance][0], dungeonDoorsTable[distance][2]);
          dungeonRenderer_drawLine(dungeonDoorsTable[distance][0], dungeonDoorsTable[distance][2], dungeonDoorsTable[distance][1], dungeonDoorsTable[distance][3]);
          dungeonRenderer_drawLine(dungeonDoorsTable[distance][1], dungeonDoorsTable[distance][3], dungeonDoorsTable[distance][1], dungeonDoorsTable[distance][5]);
        } else {
          dungeonRenderer_drawLine(0, dungeonDoorsTable[distance][2] - 3, dungeonDoorsTable[distance][1], dungeonDoorsTable[distance][3]);
          dungeonRenderer_drawLine(dungeonDoorsTable[distance][1], dungeonDoorsTable[distance][3], dungeonDoorsTable[distance][1], dungeonDoorsTable[distance][5]);
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
      if (rightTile == 4) {
        if (distance > 0) {
          dungeonRenderer_drawLine(279 - dungeonDoorsTable[distance][0], dungeonDoorsTable[distance][4], 279 - dungeonDoorsTable[distance][0], dungeonDoorsTable[distance][2]);
          dungeonRenderer_drawLine(279 - dungeonDoorsTable[distance][0], dungeonDoorsTable[distance][2], 279 - dungeonDoorsTable[distance][1], dungeonDoorsTable[distance][3]);
          dungeonRenderer_drawLine(279 - dungeonDoorsTable[distance][1], dungeonDoorsTable[distance][3], 279 - dungeonDoorsTable[distance][1], dungeonDoorsTable[distance][5]);
        } else {
          dungeonRenderer_drawLine(279, dungeonDoorsTable[distance][2] - 3, 279 - dungeonDoorsTable[distance][1], dungeonDoorsTable[distance][3]);
          dungeonRenderer_drawLine(279 - dungeonDoorsTable[distance][1], dungeonDoorsTable[distance][3], 279 - dungeonDoorsTable[distance][1], dungeonDoorsTable[distance][5]);
        }
      }
      if (centerTile == 7 || centerTile == 9) {
        dungeonRenderer_drawLine(dungeonTrapsTable[distance][0], dungeonTrapsTable[distance][4], dungeonTrapsTable[distance][2], dungeonTrapsTable[distance][5]);
        dungeonRenderer_drawLine(dungeonTrapsTable[distance][2], dungeonTrapsTable[distance][5], dungeonTrapsTable[distance][3], dungeonTrapsTable[distance][5]);
        dungeonRenderer_drawLine(dungeonTrapsTable[distance][3], dungeonTrapsTable[distance][5], dungeonTrapsTable[distance][1], dungeonTrapsTable[distance][4]);
        dungeonRenderer_drawLine(dungeonTrapsTable[distance][1], dungeonTrapsTable[distance][4], dungeonTrapsTable[distance][0], dungeonTrapsTable[distance][4]);
      }
      if (centerTile == 8) {
        dungeonRenderer_drawLine(dungeonTrapsTable[distance][0], 158 - dungeonTrapsTable[distance][4], dungeonTrapsTable[distance][2], 158 - dungeonTrapsTable[distance][5]);
        dungeonRenderer_drawLine(dungeonTrapsTable[distance][2], 158 - dungeonTrapsTable[distance][5], dungeonTrapsTable[distance][3], 158 - dungeonTrapsTable[distance][5]);
        dungeonRenderer_drawLine(dungeonTrapsTable[distance][3], 158 - dungeonTrapsTable[distance][5], dungeonTrapsTable[distance][1], 158 - dungeonTrapsTable[distance][4]);
        dungeonRenderer_drawLine(dungeonTrapsTable[distance][1], 158 - dungeonTrapsTable[distance][4], dungeonTrapsTable[distance][0], 158 - dungeonTrapsTable[distance][4]);
      }
      if (centerTile == 7 || centerTile == 8) {
        int ba = dungeonLaddersTable[distance][3];
        int tp = dungeonLaddersTable[distance][2];
        if (player.dy == 0) {
          dungeonRenderer_drawLine(139,ba,139,tp);
          dungeonRenderer_drawLine(140,tp,140,ba);
        } else {
          int lx = dungeonLaddersTable[distance][0];
          int rx = dungeonLaddersTable[distance][1];
          dungeonRenderer_drawLine(lx,ba,lx,tp);
          dungeonRenderer_drawLine(rx,tp,rx,ba);

          int y1 = (ba * 4 + tp) / 5;
          int y2 = (ba * 3 + tp * 2) / 5;
          int y3 = (ba * 2 + tp * 3) / 5;
          int y4 = (ba + tp * 4) / 5;

          dungeonRenderer_drawLine(lx,y1,rx,y1);
          dungeonRenderer_drawLine(lx,y2,rx,y2);
          dungeonRenderer_drawLine(lx,y3,rx,y3);
          dungeonRenderer_drawLine(lx,y4,rx,y4);
        }
      }
      if (centerTile == 5) {
        int t2 = distance + 1;
        dungeonRenderer_drawLine(139-20/t2,dungeonTable[distance][3],139-20/t2,dungeonTable[distance][3]-20/t2);
        dungeonRenderer_drawLine(139-20/t2,dungeonTable[distance][3]-20/t2,139+20/t2,dungeonTable[distance][3]-20/t2);
        dungeonRenderer_drawLine(139+20/t2,dungeonTable[distance][3]-20/t2,139+20/t2,dungeonTable[distance][3]);
        dungeonRenderer_drawLine(139+20/t2,dungeonTable[distance][3],139-20/t2,dungeonTable[distance][3]);
        dungeonRenderer_drawLine(139-20/t2,dungeonTable[distance][3]-20/t2,139-10/t2,dungeonTable[distance][3]-30/t2);
        dungeonRenderer_drawLine(139-10/t2,dungeonTable[distance][3]-30/t2,139+30/t2,dungeonTable[distance][3]-30/t2);
        dungeonRenderer_drawLine(139+30/t2,dungeonTable[distance][3]-30/t2,139+30/t2,dungeonTable[distance][3]-10/t2);
        dungeonRenderer_drawLine(139+30/t2,dungeonTable[distance][3]-10/t2,139+20/t2,dungeonTable[distance][3]);
        dungeonRenderer_drawLine(139+20/t2,dungeonTable[distance][3]-20/t2,139+30/t2,dungeonTable[distance][3]-30/t2);
        uiConsole_queueMessageFormat("^1%s^0", ultimaStrings[844]);
      }
    }
    int monsterInCell = (int)(dungeonMap[player.px + player.dx * distance][player.py + player.dy * distance] / 100);
    if (monsterInCell < 1) {
      if (shouldBreak) {
        break;
      }
    } else {
      int B = 79 + dungeonEnemiesHeight[distance-1][0];
      int C = 139;
      int L = C - l1;
      int R = r1 - C;
      int H = 2 * dungeonEnemiesHeight[distance-1][0];

      if (monsterInCell + monstersIndex == 32) {
        uiConsole_queueMessageFormat("^1%s^0", ultimaStrings[845]);
      } else if (distance == 0 && monsterInCell + monstersIndex != 41) {
        break;
      } else {
        uiConsole_queueMessageFormat("^1%s^0", enemyDefinitions[monsterInCell + monstersIndex].name);

        int linesCount = dungeonEnemyHplotPoints[9].hplotListCount;
        for (int i=0;i<linesCount;i++) {
          int pointCount = dungeonEnemyHplotPoints[9].hplotLists[i].pointCount;
          int x1 = C + dungeonEnemyHplotPoints[9].hplotLists[i].points[0] * (dungeonEnemyHplotPoints[9].hplotLists[i].points[0] < 0 ? L : R);
          int y1 = B + dungeonEnemyHplotPoints[9].hplotLists[i].points[1] * H;

          if (pointCount == 2) {
            dungeonRenderer_setPixel(x1, y1, 255, 255, 255);
            continue;
          }
          
          for (int j=2;j<pointCount;j+=2) {
            int x2 = C + dungeonEnemyHplotPoints[9].hplotLists[i].points[j] * (dungeonEnemyHplotPoints[9].hplotLists[i].points[j] < 0 ? L : R);
            int y2 = B + dungeonEnemyHplotPoints[9].hplotLists[i].points[j + 1] * H;
            dungeonRenderer_drawLine(x1, y1, x2, y2);
            x1 = x2;
            y1 = y2;
          }
        }

        if (monsterInCell + monstersIndex != 41) {
          break;
        }
      }
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