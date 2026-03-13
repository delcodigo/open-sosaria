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
static float waitingTime = 0.0f;

void playerOverworld_init() {
  geometry_setSprite(&playerOverworldGeometry, OS_TILE_WIDTH, OS_TILE_HEIGHT, 0, 0.5f, 0.125f, 1.0f);
  matrix4_setIdentity(transformationMatrix);
}

bool playerOverworld_updateWait() {
  if (input.space) {
    waitingTime = 0.0f;
    char waitCommand[30] = {0};
    snprintf(waitCommand, sizeof(waitCommand), "%.14s%.15s", ultimaStrings[114], ultimaStrings[115]);
    uiConsole_replaceLastMessage(waitCommand);
    keyRepeatDelay = 0.3f;
    return true;
  }

  return false;
}

bool playerOverworld_updateMovement(float deltaTime) {
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

    uiConsole_replaceLastMessage(movementCommand);

    if (tile == 0) {
      uiConsole_addMessage(ultimaStrings[138]);
      keyRepeatDelay = 0.3f;
      return true;
    } else if (tile == 3) {
      uiConsole_addMessage(ultimaStrings[139]);
      keyRepeatDelay = 0.3f;
      return true;
    }

    player.tx += moveX;
    player.ty += moveY;
    keyRepeatDelay = 0.1f;

    player_consumeFood();

    return true;
  } else {
    waitingTime += deltaTime;
    if (waitingTime >= 5.0f) {
      waitingTime = 0.0f;
      snprintf(movementCommand, sizeof(movementCommand), "%.14s%.15s", ultimaStrings[114], ultimaStrings[115]);
      uiConsole_replaceLastMessage(movementCommand);
      player_waitPenalty();
      return true;
    }
  }

  return false;
}

bool playerOverworld_update(float deltaTime) {
  bool acted = false;

  if (keyRepeatDelay <= 0) {
    if (playerOverworld_updateWait()) { acted = true; } else
    if (playerOverworld_updateMovement(deltaTime)) { acted = true; }
  } else {
    keyRepeatDelay -= deltaTime;
    if (keyRepeatDelay < 0) {
      keyRepeatDelay = 0;
    }
  }
  
  matrix4_setPosition(transformationMatrix, player.tx * OS_TILE_WIDTH, player.ty * OS_TILE_HEIGHT, 1);
  camera_setPosition3f(&camera, (player.tx + 1) * OS_TILE_WIDTH - OS_SCREEN_WIDTH / 2, (player.ty + 1) * OS_TILE_HEIGHT - OS_SCREEN_HEIGHT / 2, 10);
  float *viewMatrix = camera_getViewProjectionMatrix(&camera);

  geometry_render(&playerOverworldGeometry, ultimaAssets.overworldTiles.textureId, transformationMatrix, viewMatrix);

  return acted;
}

void playerOverworld_free() {
  geometry_free(&playerOverworldGeometry);
}