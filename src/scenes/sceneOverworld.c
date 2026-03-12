#include "engine/geometry.h"
#include "engine/camera.h"
#include "maths/matrix4.h"
#include "sceneOverworld.h"
#include "sceneDiskLoader.h"

static Geometry overworldGeometry;
static float transformMatrix[16];

static void sceneOverworld_init() {
  geometry_setSprite(&overworldGeometry, 112, 32);
  matrix4_setIdentity(transformMatrix);
  matrix4_setPosition(transformMatrix, 0, 32, 0);
}

static void sceneOverworld_update(float deltaTime) {
  (void) deltaTime;

  geometry_render(&overworldGeometry, ultimaAssets.overworldTiles.textureId, transformMatrix, camera_getViewProjectionMatrix(&camera));
}

static void sceneOverworld_free() {
}

Scene sceneOverworld = {
  .scene_init = sceneOverworld_init,
  .scene_update = sceneOverworld_update,
  .scene_free = sceneOverworld_free
};