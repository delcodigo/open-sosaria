#include <stdio.h>
#include "playerOverworld.h"
#include "engine/geometry.h"
#include "engine/camera.h"
#include "engine/input.h"
#include "ui/uiConsole.h"
#include "scenes/sceneDiskLoader.h"
#include "maths/matrix4.h"
#include "config.h"

static Geometry playerOverworldGeometry;
static float transformationMatrix[16];

static float keyRepeatDelay = 0;

void playerOverworld_init() {
  geometry_setSprite(&playerOverworldGeometry, OS_TILE_WIDTH, OS_TILE_HEIGHT, 0, 0.5f, 0.125f, 1.0f);
  matrix4_setIdentity(transformationMatrix);
}

bool playerOverworld_updateMovement(float deltaTime) {
  if (keyRepeatDelay > 0) {
    keyRepeatDelay -= deltaTime;
    return false;
  }
  
  int moveX = 0;
  int moveY = 0;
  char movementCommand[30] = {0};

  if (input.up) {
    moveY = -1;
    snprintf(movementCommand, sizeof(movementCommand), "%.14s%.15s", ultimaStrings[114], ultimaStrings[133]);
  } else if (input.down) {
    moveY = 1;
    snprintf(movementCommand, sizeof(movementCommand), "%.14s%.15s", ultimaStrings[114], ultimaStrings[134]);
  } else if (input.left) {
    moveX = -1;
    snprintf(movementCommand, sizeof(movementCommand), "%.14s%.15s", ultimaStrings[114], ultimaStrings[136]);
  } else if (input.right) {
    moveX = 1;
    snprintf(movementCommand, sizeof(movementCommand), "%.14s%.15s", ultimaStrings[114], ultimaStrings[135]);
  }

  if (moveX != 0 || moveY != 0) {
    int tileIndex = (player.ty + moveY) * OS_BTERRA_MAP_WIDTH + (player.tx + moveX);
    int world = tileIndex / (OS_BTERRA_MAP_WIDTH * OS_BTERRA_MAP_HEIGHT);
    int tile = ultimaAssets.bterraMaps[world][player.ty + moveY][player.tx + moveX];

    if (tile == 0 || tile == 3) {
      return false;
    }

    player.tx += moveX;
    player.ty += moveY;
    keyRepeatDelay = 0.1f;

    uiConsole_addMessage(movementCommand);
    player_consumeFood();

    return true;
  }

  return false;
}

bool playerOverworld_update(float deltaTime) {
  bool acted = false;
  if (playerOverworld_updateMovement(deltaTime)) { acted = true; }
  
  matrix4_setPosition(transformationMatrix, player.tx * OS_TILE_WIDTH, player.ty * OS_TILE_HEIGHT, 1);
  camera_setPosition3f(&camera, (player.tx + 1) * OS_TILE_WIDTH - OS_SCREEN_WIDTH / 2, (player.ty + 1) * OS_TILE_HEIGHT - OS_SCREEN_HEIGHT / 2, 10);
  float *viewMatrix = camera_getViewProjectionMatrix(&camera);

  geometry_render(&playerOverworldGeometry, ultimaAssets.overworldTiles.textureId, transformationMatrix, viewMatrix);

  return acted;
}

void playerOverworld_free() {
  geometry_free(&playerOverworldGeometry);
}