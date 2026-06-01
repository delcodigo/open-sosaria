#include <math.h>
#include "playerSpace.h"
#include "data/player.h"
#include "data/bevery.h"
#include "engine/engine.h"
#include "engine/input.h"
#include "playerCommons.h"
#include "entities/ui/uiConsole.h"
#include "scenes/sceneSpace.h"
#include "scenes/sceneDiskLoader.h"

static float playerTransformMatrix[16];
static Geometry thrustGeometry;
static int thrustOffset[2] = {0};
static float thrustVisible = 0;

Vector2f targetCentre = {0};

void playerSpace_init() {
  player.px = 5;
  player.py = 5;
  player.sx = 5;
  player.sy = 5;
  player.dx = 0;
  player.dy = 0;
  player.shield = 1000;
  player.fuel = 1000;
  player.rotation = 48;
  player.isDocked = false;
  if (player.vehicle == 7) {
    player.fuel = 1500;
  } else if (player.vehicle == 8) {
    player.shield = 2000;
  }

  playerState = PLAYER_STATE_IDLE;
  keyRepeatDelay = 0;

  float tx1 = 6.0f * 24.0f / (float) ultimaAssets.spaceSprites.width;
  float ty1 = 2.0f * 24.0f / (float) ultimaAssets.spaceSprites.height;
  float tx2 = tx1 + 24.0f / (float) ultimaAssets.spaceSprites.width;
  float ty2 = ty1 + 24.0f / (float) ultimaAssets.spaceSprites.height;

  geometry_setSpriteOffset(&thrustGeometry, 12, 12, 24, 24, tx1, ty1, tx2, ty2);
}

static bool playerSpace_updateTurning() {
  if (input.left == 1) {
    input.left = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[1056]);
    
    if (player.isDocked) {
      sceneSpace_crunchCollision();
      return true;
    }

    thrustVisible = 0;
    player.rotation -= 16;
    if (player.rotation < 0) { player.rotation = 48; }

    player.fuel -= 2;
    if (player.fuel < 0) { player.fuel = 0; }
    uiConsole_updateSpaceStats();

    return true;
  } else if (input.right == 1) {
    input.right = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[1062]);
    
    if (player.isDocked) {
      sceneSpace_crunchCollision();
      return true;
    }

    thrustVisible = 0;
    player.rotation += 16;
    if (player.rotation > 48) { player.rotation = 0; }

    player.fuel -= 2;
    if (player.fuel < 0) { player.fuel = 0; }
    uiConsole_updateSpaceStats();

    return true;
  }

  return false;
}

static bool playerSpace_updateThrusting() {
  if (input.up == 1) {
    input.up = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[1058]);

    if (player.isDocked) {
      sceneSpace_crunchCollision();
      return true;
    }

    int xx = 0;
    int yy = 0;
    if (player.rotation == 0) {
      yy = 8;
    } else if (player.rotation == 16) {
      xx = -8;
    } else if (player.rotation == 32) {
      yy = -8;
    } else if (player.rotation == 48) {
      xx = 8;
    }

    thrustOffset[0] = xx;
    thrustOffset[1] = yy;
    thrustVisible = 0.075f;

    player.dx -= (xx > 0) ? 1 : (xx < 0) ? -1 : 0;
    player.dy -= (yy > 0) ? 1 : (yy < 0) ? -1 : 0;

    if (player.dx > 5) { player.dx = 5; } else
    if (player.dy < -5) { player.dy = -5; } else
    if (player.dy > 5) { player.dy = 5; } else
    if (player.dx < -5) { player.dx = -5; } 

    player.fuel -= 5;
    if (player.fuel < 0) { player.fuel = 0; }
    uiConsole_updateSpaceStats();

    return true;
  }

  return false;
}

static bool playerSpace_updateRetro() {
  if (input.down == 1) {
    input.down = 2;

    player.isDocked = false;
    
    int xx = 0;
    int yy = 0;

    if (player.rotation == 0) {
      yy = -1;
    } else if (player.rotation == 16) {
      xx = 1;
    } else if (player.rotation == 32) {
      yy = 1;
    } else if (player.rotation == 48) {
      xx = -1;
    }

    player.dx -= xx;
    if (player.dx > 5) { player.dx = 5; }
    if (player.dx < -5) { player.dx = -5; }

    player.dy -= yy;
    if (player.dy > 5) { player.dy = 5; }
    if (player.dy < -5) { player.dy = -5; }

    player.fuel -= 5;
    if (player.fuel < 0) { player.fuel = 0; }
    uiConsole_updateSpaceStats();

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[1064]);

    return true;
  }
  return false;
}

static void playerSpace_updateMovement(float deltaTime) {
  if (player.dx == 0 && player.dy == 0) { return; }

  float speed = 3.0f * deltaTime;
  player.sx += (float) player.dx * speed;
  player.sy += (float) player.dy * speed;

  if (player.sx < 15) { player.sx = 265; } else
  if (player.sx > 265) { player.sx = 15; }

  if (player.sy < 10) { player.sy = 150; } else
  if (player.sy > 150) { player.sy = 10; }

  sceneSpace_checkCollisions();
}

static bool playerSpace_updateSpaceStation() {
  int shapeId = 0;
  int rotation = 0;

  if (input.up == 1) {
    input.up = 2;
    shapeId = 7;
    rotation = 32;
    uiConsole_queueMessage(ultimaStrings[1092]);
  } else if (input.left == 1) {
    input.left = 2;
    shapeId = 6;
    rotation = 16;
    uiConsole_queueMessage(ultimaStrings[1091]);
  } else if (input.down == 1) {
    input.down = 2;
    shapeId = 5;
    rotation = 0;
    uiConsole_queueMessage(ultimaStrings[1090]);
  } else if (input.right == 1) {
    input.right = 2;
    shapeId = 8;
    rotation = 48;
    uiConsole_queueMessage(ultimaStrings[1093]);
  } else {
    return false;
  }

  return sceneSpace_tryBoardVessel(shapeId, rotation);
}

static bool playerSpace_updateBoard() {
  if (input.b == 1) {
    input.b = 2;

    uiConsole_replaceLastMessageFormat("%s%s%s", ultimaStrings[98], ultimaStrings[1052], vehicleNames[player.vehicle]);
    uiConsole_queueMessage(ultimaStrings[1098]);

    return true;
  }

  return false;
}

static bool playerSpace_updateNoFuelDrift() {
  if (lastKey != 0 && player.fuel <= 0) {
    lastKey = 0;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[1046]);
    uiConsole_queueMessage(ultimaStrings[1047]);

    return true;
  }

  return false;
}

static bool playerSpace_updateInfo() {
  if (input.i == 1) {
    input.i = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[1031]);
    uiConsole_queueMessage(ultimaStrings[1032]);

    int px = 0;
    int py = 0;
    
    for (int yy=1;yy<=3;yy++) {
      char line[4] = {0};
      int lineIndex = 0;

      for (int xx=1;xx<=3;xx++) {
        px = player.px - 2 + xx;
        py = player.py - 2 + yy;
        float sector = spaceMap[px][py] + 32767;

        if (floorf((sector - pow(4, 8) * floorf(sector / pow(4, 8))) / pow(4, 7)) > 0) {
          line[lineIndex++] = '^';
        } else if (floorf((sector - pow(4, 3) * floorf(sector / pow(4, 3))) / pow(4, 2)) > 0) {
          line[lineIndex++] = 'B';
        } else if (floorf((sector - pow(4, 2) * floorf(sector / pow(4, 2))) / 4.0f) > 0) {
          line[lineIndex++] = 'O';
        } else if (floorf(sector - 4 * floorf(sector / 4.0f)) > 0) {
          line[lineIndex++] = '*';
        } else {
          line[lineIndex++] = '-';
        }
      }

      uiConsole_queueMessage(line);
    }

    return true;
  }

  return false;
}

static bool playerSpace_updateViewChange() {
  if (input.v == 1) {
    input.v = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[1049]);

    if (isFirstPersonView && enemyCrafts > 0) {
      uiConsole_queueMessage(ultimaStrings[1039]);
      uiConsole_queueMessage(ultimaStrings[1040]);
      return true;
    }

    if (player.isDocked) {
      uiConsole_queueMessage(ultimaStrings[1050]);
      uiConsole_queueMessage(ultimaStrings[1051]);

      return true;
    }

    uiConsole_queueMessage(ultimaStrings[1022]);

    if (!isFirstPersonView) {
      isFirstPersonView = true;
      playerState = PLAYER_STATE_SPACE_FIRST_PERSON;
    } else {
      isFirstPersonView = false;
      playerState = PLAYER_STATE_IDLE;
    }

    return true;
  }

  return false;
}

static bool playerSpace_update3DMovement() {
  if (input.up == 1) {
    input.up = 2;
    targetCentre.y = 40;
    return true;
  } else if (input.down == 1) {
    input.down = 2;
    targetCentre.y = 88;
    return true;
  } else if (input.left == 1) {
    input.left = 2;
    targetCentre.x = 80;
    return true;
  } else if (input.right == 1) {
    input.right = 2;
    targetCentre.x = 176;
    return true;
  } else if (input.space == 1) {
    input.space = 2;
    targetCentre.x = 128;
    targetCentre.y = 64;
    return true;
  }

  return false;
}

bool playerSpace_update(float deltaTime) {
  bool acted = false;
  
  if (keyRepeatDelay <= 0) {
    switch (playerState) {
      case PLAYER_STATE_IDLE:
        if (playerSpace_updateNoFuelDrift()) { acted = true; } else
        if (playerSpace_updateBoard()) { acted = true; } else
        if (playerSpace_updateInfo()) { acted = true; } else
        if (playerSpace_updateTurning()) { acted = true; } else 
        if (playerSpace_updateViewChange()) { acted = true; } else 
        if (playerSpace_updateThrusting()) { acted = true; } else 
        if (playerSpace_updateRetro()) { acted = true; }
        playerSpace_updateMovement(deltaTime);
        break;
      
      case PLAYER_STATE_SPACE_STATION:
        if (playerSpace_updateSpaceStation()) { acted = true; }
        break;
      
      case PLAYER_STATE_SPACE_FIRST_PERSON:
        if (playerSpace_update3DMovement()) { acted = true; } else
        if (playerSpace_updateViewChange()) { acted = true; } 
        break;
      
      default:
        break;
    }
  } else {
    keyRepeatDelay -= deltaTime;
    if (keyRepeatDelay < 0) {
      keyRepeatDelay = 0;
    }
  }

  if (thrustVisible > 0) {
    thrustVisible -= deltaTime;
  }

  return acted;
}

void playerSpace_render(float *viewMatrix) {
  int playerShip = 0;
  if (player.vehicle == 7) { playerShip = 2; } else
  if (player.vehicle == 8) { playerShip = 1; }

  if (thrustVisible > 0) {
    sceneSpace_transformShape(playerTransformMatrix, (int) (player.sx + thrustOffset[0]), (int) (player.sy + thrustOffset[1]), player.rotation);
    geometry_render(&thrustGeometry, ultimaAssets.spaceSprites.textureId, playerTransformMatrix, viewMatrix);
  }

  sceneSpace_transformShape(playerTransformMatrix, (int) player.sx, (int) player.sy, player.rotation);
  geometry_render(&playerShipGeometries[playerShip], ultimaAssets.spaceSprites.textureId, playerTransformMatrix, viewMatrix);
}

void playerSpace_free() {
  geometry_free(&thrustGeometry);
}