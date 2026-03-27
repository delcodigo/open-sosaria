#include "playerTown.h"
#include "engine/geometry.h"
#include "engine/input.h"
#include "entities/ui/uiConsole.h"
#include "scenes/sceneDiskLoader.h"
#include "scenes/sceneOverworld.h"
#include "scenes/sceneTown.h"
#include "data/player.h"
#include "data/bevery.h"
#include "maths/matrix4.h"
#include "playerCommons.h"
#include "vmExecuter.h"
#include "config.h"

static Geometry playerTownGeometry;
static float transformMatrix[16];

void playerTown_init() {
  float tx1 = (6.0f * OS_TOWN_CASTLE_SPRITE_WIDTH) / (float)ultimaAssets.townCastleSprites.width;
  float tx2 = (7.0f * OS_TOWN_CASTLE_SPRITE_WIDTH) / (float)ultimaAssets.townCastleSprites.width;

  geometry_setSprite(&playerTownGeometry, OS_TOWN_CASTLE_SPRITE_WIDTH, OS_TOWN_CASTLE_SPRITE_HEIGHT, tx1, 0, tx2, 1);
}

static bool playerTown_checkExit(int moveY) {
  if (player.py + moveY > 21) {
    vmExecuter_createSceneTransition(0.5f, &sceneOverworld);
    return true;
  }

  return false;
}

static bool playerTown_updateMovement(float deltaTime) {
  int moveX = 0;
  int moveY = 0;
  int movementStringIndex = -1;

  if (input.up) {
    moveY = -1;
    movementStringIndex = 117;
  } else if (input.down) {
    moveY = 1;
    movementStringIndex = 118;
  } else if (input.left) {
    moveX = -1;
    movementStringIndex = 120;
  } else if (input.right) {
    moveX = 1;
    movementStringIndex = 119;
  }

  if (moveX != 0 || moveY != 0) {
    waitingTime = 0.0f;
    uiConsole_replaceLastMessageFormat("%.14s%.15s", ultimaStrings[98], ultimaStrings[movementStringIndex]);

    if (sceneTown_isSolid(player.px + moveX, player.py + moveY)) {
      uiConsole_addMessage(ultimaStrings[341]);
      keyRepeatDelay = 0.3f;
      return true;
    }
    
    if (playerTown_checkExit(moveY)) {
      return true;
    }

    player.px = player.px + moveX;
    player.py = player.py + moveY;
    keyRepeatDelay = 0.1f;

    player_consumeTownFood();

    return true;
  } else {
    waitingTime += deltaTime;
    if (waitingTime >= 5.0f) {
      waitingTime = 0.0f;
      uiConsole_replaceLastMessageFormat("%.14s%.15s", ultimaStrings[98], ultimaStrings[99]);
      player_waitPenalty();
      return true;
    }
  }
  
  return false;
}

static bool playerTown_updateCast() {
  if (input.c == 1) {
    input.c = 2;
    waitingTime = 0.0f;

    uiConsole_replaceLastMessageFormat("%.15s%.15s", ultimaStrings[98], ultimaStrings[359]);
    uiConsole_queueMessage(ultimaStrings[360]);
    uiConsole_queueMessage(ultimaStrings[361]);

    return true;
  }

  return false;
}

static bool playerTown_updateGet() {
  if (input.g == 1) {
    input.g = 2;
    waitingTime = 0.0f;

    uiConsole_replaceLastMessageFormat("%.14s%.15s", ultimaStrings[98], ultimaStrings[383]);

    return true;
  }

  return false;
}

static bool playerTown_updateInfo() {
  if (input.i == 1) {
    input.i = 2;
    waitingTime = 0.0f;

    int tx = (int)(player.tx % OS_BTERRA_MAP_WIDTH);
    int ty = (int)(player.ty % OS_BTERRA_MAP_HEIGHT);
    int world = ((int)player.ty / OS_BTERRA_MAP_HEIGHT) * 2 + ((int)player.tx / OS_BTERRA_MAP_WIDTH);
    int tileType = ultimaAssets.bterraMaps[world][ty][tx] & 0x0F;

    uiConsole_replaceLastMessageFormat("%.14s%.15s", ultimaStrings[98], ultimaStrings[384]);
    uiConsole_queueMessage(ultimaStrings[385]);
    uiConsole_queueMessageFormat("%.15s%.15s", ultimaStrings[386], placesNames[world * 20 + tileType + 13]);
    uiConsole_queueMessage(ultimaStrings[387]);

    return true;
  }

  return false;
}

bool playerTown_updateSave() {
  if (input.q == 1) {
    input.q = 2;
    uiConsole_replaceLastMessageFormat("%.14s%.15s", ultimaStrings[98], ultimaStrings[389]);
    uiConsole_queueMessage(ultimaStrings[390]);
    return true;
  }

  return false;
}

bool playerTown_update(float deltaTime) {
  bool acted = false;
  
  if (keyRepeatDelay <= 0) {
    if (playerCommons_updateZtats()) { acted = true; } else
    if (playerCommons_updateWait()) { acted = true; } else
    if (playerCommons_updateReady()) { acted = true; } else
    if (playerTown_updateCast()) { acted = true; } else
    if (playerTown_updateGet()) { acted = true; } else
    if (playerTown_updateInfo()) { acted = true; } else
    if (playerTown_updateSave()) { acted = true; } else
    if (playerTown_updateMovement(deltaTime)) { acted = true; }
  } else {
    keyRepeatDelay -= deltaTime;
    if (keyRepeatDelay < 0) {
      keyRepeatDelay = 0;
    }
  }

  return acted;
}

void playerTown_render(float *viewMatrix) {
  matrix4_setIdentity(transformMatrix);
  matrix4_setPosition(transformMatrix, player.px * OS_TOWN_CASTLE_SPRITE_WIDTH, player.py * OS_TOWN_CASTLE_SPRITE_HEIGHT, 1);

  geometry_render(&playerTownGeometry, ultimaAssets.townCastleSprites.textureId, transformMatrix, viewMatrix);
}

void playerTown_free() {
  geometry_free(&playerTownGeometry);
}