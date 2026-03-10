#include <string.h>
#include "sceneSplash.h"
#include "engine/geometry.h"
#include "config.h"
#include "sceneDiskLoader.h"
#include "maths/matrix4.h"

Geometry splashGeometry;
float transformMatrix[16];

void sceneSplash_init() {
  geometry_setSprite(&splashGeometry, OS_SCREEN_WIDTH, OS_SCREEN_HEIGHT);
  matrix4_setIdentity(transformMatrix);
}

void sceneSplash_update(float deltaTime) {
  (void) deltaTime;
  geometry_render(&splashGeometry, ultimaAssets.titleScreen.textureId, transformMatrix, camera_getViewProjectionMatrix(&camera));
}

void sceneSplash_free() {
  geometry_free(&splashGeometry);
}

Scene sceneSplash = {
  .scene_init = sceneSplash_init,
  .scene_update = sceneSplash_update,
  .scene_free = sceneSplash_free
};