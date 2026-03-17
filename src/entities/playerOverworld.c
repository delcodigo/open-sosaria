#include <stdio.h>
#include <string.h>
#include "playerOverworld.h"
#include "engine/geometry.h"
#include "engine/camera.h"
#include "engine/input.h"
#include "data/saveAndLoad.h"
#include "data/bevery.h"
#include "ui/uiConsole.h"
#include "ui/uiztats.h"
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
    snprintf(waitCommand, sizeof(waitCommand), "%.14s%.15s", ultimaStrings[98], ultimaStrings[99]);
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
    snprintf(movementCommand, sizeof(movementCommand), "%.14s%.15s", ultimaStrings[98], ultimaStrings[117]);
  } else if (input.down) {
    moveY = 1;
    snprintf(movementCommand, sizeof(movementCommand), "%.14s%.15s", ultimaStrings[98], ultimaStrings[118]);
  } else if (input.left) {
    moveX = -1;
    snprintf(movementCommand, sizeof(movementCommand), "%.14s%.15s", ultimaStrings[98], ultimaStrings[120]);
  } else if (input.right) {
    moveX = 1;
    snprintf(movementCommand, sizeof(movementCommand), "%.14s%.15s", ultimaStrings[98], ultimaStrings[119]);
  }

  if (moveX != 0 || moveY != 0) {
    int tx = (int)((player.tx + moveX) % OS_BTERRA_MAP_WIDTH);
    int ty = (int)((player.ty + moveY) % OS_BTERRA_MAP_HEIGHT);
    int world = ((int)(player.ty + moveY) / OS_BTERRA_MAP_HEIGHT) * 2 + ((int)(player.tx + moveX) / OS_BTERRA_MAP_WIDTH);
    int tile = (ultimaAssets.bterraMaps[world][ty][tx] >> 4) & 0x0F;

    uiConsole_replaceLastMessage(movementCommand);

    if (tile == 0) {
      uiConsole_addMessage(ultimaStrings[122]);
      keyRepeatDelay = 0.3f;
      return true;
    } else if (tile == 3) {
      uiConsole_addMessage(ultimaStrings[123]);
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
      snprintf(movementCommand, sizeof(movementCommand), "%.14s%.15s", ultimaStrings[98], ultimaStrings[99]);
      uiConsole_replaceLastMessage(movementCommand);
      player_waitPenalty();
      return true;
    }
  }

  return false;
}

static bool playerOverworld_updateZtats() {
  if (input.z == 1) {
    input.z = 2;
    uiZtats_init();
    ztatsActive = true;
    waitingTime = 0.0f;
    memset(&input, 0, sizeof(input));
    return true;
  }

  return false;
}

static bool playerOverworld_updateSave() {
  if (input.q == 1) {
    input.q = 2;
    uiConsole_addMessage(ultimaStrings[190]);
    waitingTime = 0.0f;
    memset(&input, 0, sizeof(input));
    saveGame();
    uiConsole_addMessage(ultimaStrings[197]);
    return true;
  }

  return false;
}

static bool playerOverworld_updateInfo() {
  if (input.i == 1) {
    input.i = 2;
    waitingTime = 0.0f;
    memset(&input, 0, sizeof(input));

    uiConsole_addMessage(ultimaStrings[176]);

    int tx = (int)(player.tx % OS_BTERRA_MAP_WIDTH);
    int ty = (int)(player.ty % OS_BTERRA_MAP_HEIGHT);
    int world = ((int)player.ty / OS_BTERRA_MAP_HEIGHT) * 2 + ((int)player.tx / OS_BTERRA_MAP_WIDTH);
    int tile = (ultimaAssets.bterraMaps[world][ty][tx] >> 4) & 0x0F;
    int tileType = ultimaAssets.bterraMaps[world][ty][tx] & 0x0F;

    if (tile == 0) { uiConsole_addMessage(ultimaStrings[177]); } else
    if (tile == 1) { uiConsole_addMessage(ultimaStrings[178]); } else
    if (tile == 2) { uiConsole_addMessage(ultimaStrings[179]); } else 
    if (tile == 4) { uiConsole_addMessage(placesNames[world * 20 + tileType + 1]); } else
    if (tile == 5) { uiConsole_addMessage(placesNames[world * 20 + tileType + 3]); } else
    if (tile == 6) { 
      char placeName[41] = {0};
      snprintf(placeName, sizeof(placeName), "%s%s", ultimaStrings[180], placesNames[world * 20 + tileType + 13]);
      uiConsole_addMessage(placeName);
    } else
    if (tile == 7) { uiConsole_addMessage(placesNames[world * 20 + tileType + 5]); }

    uiConsole_addMessage(ultimaStrings[181]);

    return true;
  }

  return false;
}

bool playerOverworld_update(float deltaTime) {
  bool acted = false;

  if (keyRepeatDelay <= 0) {
    if (playerOverworld_updateZtats()) { acted = true; } else
    if (playerOverworld_updateWait()) { acted = true; } else
    if (playerOverworld_updateSave()) { acted = true; } else 
    if (playerOverworld_updateInfo()) { acted = true; } else
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