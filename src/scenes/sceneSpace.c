#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "sceneSpace.h"
#include "engine/geometry.h"
#include "entities/ui/uiConsole.h"
#include "sceneDiskLoader.h"
#include "sceneOverworld.h"
#include "entities/playerSpace.h"
#include "entities/vmExecuter.h"
#include "data/player.h"
#include "maths/matrix4.h"
#include "utils.h"

#define SHAPE_SPACE_SHUTTLE 15
#define SHAPE_SPACE_CRUISER 16
#define SHAPE_SPACE_ZEPHER 17
#define SHAPE_SPACE_PLANET 18
#define SHAPE_TIME_MACHINE 19
#define SHAPE_SPACE_STATION 20
#define SHAPE_SPACE_STAR 21

static SpaceShape shapes[11];
static bool darkDeath = false;

Geometry playerShipGeometries[3];
int spaceMap[11][11];

static int sceneSpace_getBase4Digit(int position, int value) {
  return ((int)(value / pow(4, position)) - 4 * (int)(value / pow(4, position + 1)));
}

static int sceneSpace_getRandomX() {
  return (int)(rand01() * 220 + 30);
}

static int sceneSpace_getRandomY() {
  return (int)(rand01() * 100 + 30);
}

static float sceneSpace_getDistanceBetweenShapes(SpaceShape *shape1, SpaceShape *shape2) {
  float dx = shape1->x - shape2->x;
  float dy = shape1->y - shape2->y;

  return sqrtf(dx*dx + dy*dy);
}

static void sceneSpace_setShapeGeometry(int shapeId, Geometry *geometry) {
  float tx1 = ((shapeId % 8) * 24.0f + 5.0f) / ultimaAssets.spaceSprites.width;
  float ty1 = (floor(shapeId / 8.0f) * 24.0f + 5.0f) / ultimaAssets.spaceSprites.height;
  float tx2 = tx1 + 15.0f / (float) ultimaAssets.spaceSprites.width;
  float ty2 = ty1 + 15.0f / (float) ultimaAssets.spaceSprites.height;

  geometry_setSpriteOffset(geometry, 7, 7, 15, 15, tx1, ty1, tx2, ty2);
}

static void sceneSpace_init() {
  darkDeath = false;
  playerSpace_init();

  int data[] = { 81, 49152, 17681, 49152, 0, 49152, 49152, 49152, 0, 49152 };
  for (int i=1;i<=10;i++) {
    shapes[i].x = data[i - 1];
  }

  for (int x=0;x<11;x++) {
    for (int y=0;y<11;y++) {
      spaceMap[x][y] = -32767;
    }
  }

  srand(-1);
  for (int xx=2;xx<=8;xx++) {
    for (int yy=2;yy<=8;yy++) {
      spaceMap[xx][yy] = shapes[(int)(rand01() * 10 + 1)].x - 32767;
      if (rand01() < 0.4f) {
        spaceMap[xx][yy] = -32767;
      }
    }
  }

  spaceMap[5][5] = 1301 - 32767;

  srand(-player.sx - 10 * (-player.sy));
  int zx = spaceMap[(int)player.sx][(int)player.sy] + 32767;

  // Default shape data initialization
  for (int i=0;i<=8;i++) {
    shapes[i].rotation = 0;
    shapes[i].shapeId = 0;
    shapes[i].x = -15;
    shapes[i].y = -15;
  }

  // Initialize star shape
  if (sceneSpace_getBase4Digit(0, zx) == 1) {
    shapes[0].shapeId = SHAPE_SPACE_STAR;
    shapes[0].x = sceneSpace_getRandomX();
    shapes[0].y = sceneSpace_getRandomY();
    sceneSpace_setShapeGeometry(shapes[0].shapeId, &shapes[0].geometry);
  }

  // Initialize planets shapes
  if (sceneSpace_getBase4Digit(1, zx) >= 1) { 
    for (int i=1;i<=sceneSpace_getBase4Digit(1, zx);i++) {
      shapes[i].shapeId = SHAPE_SPACE_PLANET;
      sceneSpace_setShapeGeometry(shapes[i].shapeId, &shapes[i].geometry);
      bool valid = false;
      do {
        shapes[i].x = sceneSpace_getRandomX();
        shapes[i].y = sceneSpace_getRandomY();
        valid = true;

        for (int j=0;j<=3;j++) {
          if (sceneSpace_getDistanceBetweenShapes(&shapes[i], &shapes[j]) < 45 && i != j) {
            valid = false;
            break;
          }
        }
      } while (!valid);
    }
  }

  // Initialize space station
  if (sceneSpace_getBase4Digit(2, zx) >= 1) {
    bool valid = false;
    do {
      shapes[4].shapeId = SHAPE_SPACE_STATION; 
      shapes[4].x = sceneSpace_getRandomX();
      shapes[4].y = sceneSpace_getRandomY();
      sceneSpace_setShapeGeometry(shapes[4].shapeId, &shapes[4].geometry);
      valid = true;
      for (int i=0;i<=3;i++) {
        if (sceneSpace_getDistanceBetweenShapes(&shapes[i], &shapes[4]) < 30) {
          valid = false;
          break;
        }
      }
    } while (!valid);

    int yy = 5;

    // Initialize Docked ships
    for (int xx=3;xx<=6;xx++) {
      if (sceneSpace_getBase4Digit(xx, zx) == 0) {
        continue;
      }

      int x = 0;
      for (x=yy;x<=yy+sceneSpace_getBase4Digit(xx, zx) - 1;x++) {
        if (x > 8) {
          continue;
        }

        
        if (x - 5 == 0) {
          shapes[x].rotation = 0;
          shapes[x].x = shapes[4].x;
          shapes[x].y = shapes[4].y + 14;
        } else if (x - 5 == 1) {
          shapes[x].rotation = 16;
          shapes[x].x = shapes[4].x - 14;
          shapes[x].y = shapes[4].y;
        } else if (x - 5 == 2) {
          shapes[x].rotation = 32;
          shapes[x].x = shapes[4].x;
          shapes[x].y = shapes[4].y - 14;
        } else if (x - 5 == 3) {
          shapes[x].rotation = 48;
          shapes[x].x = shapes[4].x + 14;
          shapes[x].y = shapes[4].y;
        }

        shapes[x].shapeId = xx + 12;
        if (xx == 6) {
          shapes[x].shapeId = SHAPE_TIME_MACHINE;
        }
        sceneSpace_setShapeGeometry(shapes[x].shapeId, &shapes[x].geometry);
      }

      yy = x;
    }
  }

  bool valid = false;
  do {
    player.sx = sceneSpace_getRandomX();
    player.sy = sceneSpace_getRandomY();
    shapes[9].x = player.sx;
    shapes[9].y = player.sy;
    valid = true;
    for (int i=0;i<=8;i++) {
      if (sceneSpace_getDistanceBetweenShapes(&shapes[9], &shapes[i]) < 30) {
        valid = false;
        break;
      }
    }
  } while (!valid);

  for (int i=0;i<3;i++) {
    sceneSpace_setShapeGeometry(SHAPE_SPACE_SHUTTLE + i, &playerShipGeometries[i]);
  }

  camera_setPosition3f(&camera, 0, 0, 10);
  uiConsole_setSpaceLabels();
  uiConsole_updateSpaceStats();

  uiConsole_queueMessage(ultimaStrings[1010]);
  uiConsole_queueMessage(ultimaStrings[98]);
}

void sceneSpace_transformShape(float *transformMatrix, float x, float y, float rotation) {
  matrix4_setIdentity(transformMatrix);
  matrix4_setPosition(transformMatrix, x, y, 0);
  matrix4_setRotationZ(transformMatrix, (rotation * 5.625f) * M_PI / 180.0f);
}

static void sceneSpace_crashDeath() {
  darkDeath = true;

  int htab = 19 - (strlen(player.name) + 16) / 2;

  uiConsole_addMessageFormat("%*s%s%s", htab, " ", player.name, ultimaStrings[1099]);
  uiConsole_addMessage(ultimaStrings[1100]);
  uiConsole_addMessageFormat("%*s%s", 9, " ", ultimaStrings[1101]);
  uiConsole_addMessage(" ");
}

static void sceneSpace_tryAndDock() {
  SpaceShape spaceStation = shapes[4];
  bool isDocked = false;
  int sx = (int) floorf(player.sx);
  int sy = (int) floorf(player.sy);
  int dx = player.dx;
  int dy = player.dy;
  int rt = player.rotation;

  if (sx == spaceStation.x+1 && sy >= spaceStation.y - 13 && dx == 0 && dy == 1 && rt == 32) {
    isDocked = true;
  } else if (sx == spaceStation.x && sy == spaceStation.y + 14 && dx == 0 && dy == -1 && rt == 0) {
    isDocked = true;
  } else if (sy == spaceStation.y+1 && sx == spaceStation.x + 14 && dy == 0 && dx == -1 && rt == 48) {
    isDocked = true;
  } else if (sy == spaceStation.y && sx == spaceStation.x - 13 && dy == 0 && dx == 1 && rt == 16) {
    isDocked = true;
  }

  if (isDocked) {
    uiConsole_addMessage(ultimaStrings[1043]);
    player.dx = 0;
    player.dy = 0;
    player.isDocked = true;
  } else {
    // TODO: Crunch
  }
}

void sceneSpace_checkCollisions() {
  shapes[9].x = player.sx;
  shapes[9].y = player.sy;

  for (int i=0;i<9;i++) {
    if (sceneSpace_getDistanceBetweenShapes(&shapes[i], &shapes[9]) <= 15) {
      if (shapes[i].shapeId == SHAPE_SPACE_PLANET) {
        if (player.vehicle != 6) {
          sceneSpace_crashDeath();
        } else {
          vmExecuter_createSceneTransition(1, &sceneOverworld);
        }
        return;
      } else if (shapes[i].shapeId == SHAPE_SPACE_STAR) {
        sceneSpace_crashDeath();
      } else if (shapes[i].shapeId == SHAPE_SPACE_STATION) {
        sceneSpace_tryAndDock();
      }
    }
  }
}

static void sceneSpace_render() {
  if (darkDeath) { return; }

  float *viewMatrix = camera_getViewProjectionMatrix(&camera);
  float transformMatrix[16];

  for (int i=0;i<=8;i++) {
    if (shapes[i].shapeId < 0) {
      continue;
    }

    sceneSpace_transformShape(transformMatrix, shapes[i].x, shapes[i].y, shapes[i].rotation);
    geometry_render(&shapes[i].geometry, ultimaAssets.spaceSprites.textureId, transformMatrix, viewMatrix);
  }

  playerSpace_render(viewMatrix);
}

static void sceneSpace_update(float deltaTime) {
  if (!queuedMessagesCount && lagTime <= 0 && !vmExecuter_update(deltaTime)) {
    if (playerActed) {
      uiConsole_addMessage(ultimaStrings[98]);
      playerActed = false;
    }

    if (playerSpace_update(deltaTime)) {
      playerActed = true;
    }
  }

  sceneSpace_render();
  if (darkDeath) {
    uiConsole_renderConsoleOnly();
  } else {
    uiConsole_update(deltaTime);
  }
}

static void sceneSpace_free() {
  playerSpace_free();

  for (int i=0;i<3;i++) {
    geometry_free(&playerShipGeometries[i]);
  }

  for (int i=0;i<11;i++) {
    if (shapes[i].shapeId >= 0){
      geometry_free(&shapes[i].geometry);
    }
  }
}

Scene sceneSpace = {
  .scene_init = sceneSpace_init,
  .scene_update = sceneSpace_update,
  .scene_free = sceneSpace_free
};