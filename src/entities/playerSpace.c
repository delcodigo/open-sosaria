#include "playerSpace.h"
#include "data/player.h"
#include "engine/engine.h"
#include "engine/input.h"
#include "playerCommons.h"
#include "entities/ui/uiConsole.h"
#include "scenes/sceneSpace.h"
#include "scenes/sceneDiskLoader.h"

static float playerTransformMatrix[16];

static bool playerSpace_updateTurning() {
  if (input.left == 1) {
    input.left = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[1056]);
    
    if (player.isDocked) {
      // TODO: Crunch against space station
      return true;
    }

    player.rotation -= 16;
    if (player.rotation < 0) { player.rotation = 48; }

    player.fuel -= 2;
    uiConsole_updateSpaceStats();

    return true;
  } else if (input.right == 1) {
    input.right = 2;

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[1062]);
    
    if (player.isDocked) {
      // TODO: Crunch against space station
      return true;
    }

    player.rotation += 16;
    if (player.rotation > 48) { player.rotation = 0; }

    player.fuel -= 2;
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
      // TODO: Crunch against space station
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

    // TODO: Draw thrust fire

    player.dx -= (xx > 0) ? 1 : (xx < 0) ? -1 : 0;
    player.dy -= (yy > 0) ? 1 : (yy < 0) ? -1 : 0;

    if (player.dx > 5) { player.dx = 5; } else
    if (player.dy < -5) { player.dy = -5; } else
    if (player.dy > 5) { player.dy = 5; } else
    if (player.dx < -5) { player.dx = -5; } 

    player.fuel -= 5;

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

    uiConsole_replaceLastMessageFormat("%s%s", ultimaStrings[98], ultimaStrings[1064]);

    return true;
  }
  return false;
}

bool playerSpace_update(float deltaTime) {
  bool acted = false;
  
  if (keyRepeatDelay <= 0) {
    if (playerSpace_updateTurning()) { acted = true; } else 
    if (playerSpace_updateThrusting()) { acted = true; } else 
    if (playerSpace_updateRetro()) { acted = true; }
  } else {
    keyRepeatDelay -= deltaTime;
    if (keyRepeatDelay < 0) {
      keyRepeatDelay = 0;
    }
  }

  float speed = 3.0f * deltaTime;
  player.sx += (float) player.dx * speed;
  player.sy += (float) player.dy * speed;

  return acted;
}

void playerSpace_render(float *viewMatrix) {
  int playerShip = 0;
  if (player.vehicle == 7) { playerShip = 2; } else
  if (player.vehicle == 8) { playerShip = 1; }

  sceneSpace_transformShape(playerTransformMatrix, (int) player.sx, (int) player.sy, player.rotation);
  geometry_render(&playerShipGeometries[playerShip], ultimaAssets.spaceSprites.textureId, playerTransformMatrix, viewMatrix);
}