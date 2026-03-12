#include "playerOverworld.h"
#include "engine/geometry.h"
#include "engine/camera.h"
#include "scenes/sceneDiskLoader.h"
#include "maths/matrix4.h"
#include "config.h"

static Geometry playerOverworldGeometry;
static float transformationMatrix[16];

void playerOverworld_init() {
  geometry_setSprite(&playerOverworldGeometry, OS_TILE_WIDTH, OS_TILE_HEIGHT, 0, 0.5f, 0.125f, 1.0f);
  matrix4_setIdentity(transformationMatrix);
}

bool playerOverworld_update(float deltaTime) {
  (void) deltaTime;
  
  matrix4_setPosition(transformationMatrix, player.tx * OS_TILE_WIDTH, player.ty * OS_TILE_HEIGHT, 1);
  camera_setPosition3f(&camera, (player.tx) * OS_TILE_WIDTH, -(player.ty) * OS_TILE_HEIGHT + OS_SCREEN_HEIGHT, 10);
  float *viewMatrix = camera_getViewProjectionMatrix(&camera);

  geometry_render(&playerOverworldGeometry, ultimaAssets.overworldTiles.textureId, transformationMatrix, viewMatrix);

  return true;
}

void playerOverworld_free() {
  geometry_free(&playerOverworldGeometry);
}