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

bool playerSpace_update(float deltaTime) {
  bool acted = false;
  
  if (keyRepeatDelay <= 0) {
    if (playerSpace_updateTurning()) { acted = true; }
  } else {
    keyRepeatDelay -= deltaTime;
    if (keyRepeatDelay < 0) {
      keyRepeatDelay = 0;
    }
  }

  return acted;
}

void playerSpace_render(float *viewMatrix) {
  int playerShip = 0;
  if (player.vehicle == 7) { playerShip = 2; } else
  if (player.vehicle == 8) { playerShip = 1; }

  sceneSpace_transformShape(playerTransformMatrix, player.px, player.py, player.rotation);
  geometry_render(&playerShipGeometries[playerShip], ultimaAssets.spaceSprites.textureId, playerTransformMatrix, viewMatrix);
}