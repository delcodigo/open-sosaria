#include "playerSpace.h"
#include "data/player.h"
#include "scenes/sceneSpace.h"
#include "scenes/sceneDiskLoader.h"

static float playerTransformMatrix[16];

void playerSpace_render(float *viewMatrix) {
  int playerShip = 0;
  if (player.vehicle == 7) { playerShip = 2; } else
  if (player.vehicle == 8) { playerShip = 1; }

  sceneSpace_transformShape(playerTransformMatrix, player.px, player.py, player.rotation);
  geometry_render(&playerShipGeometries[playerShip], ultimaAssets.spaceSprites.textureId, playerTransformMatrix, viewMatrix);
}
