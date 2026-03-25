#include "sceneTown.h"
#include "sceneDiskLoader.h"
#include "engine/geometry.h"
#include "maths/matrix4.h"
#include "entities/ui/uiConsole.h"
#include "config.h"

static Geometry townGeometry;
static float townTransform[16];

void sceneTown_init() {
  geometry_setSprite(&townGeometry, OS_SCREEN_WIDTH, OS_SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f);
  matrix4_setIdentity(townTransform);

  camera_setPosition3f(&camera, 0.0f, 0.0f, 10.0f);

  uiConsole_addMessage(ultimaStrings[98]);
}

void sceneTown_update(float deltaTime) {
  float *viewMatrix = camera_getViewProjectionMatrix(&camera);
  geometry_render(&townGeometry, ultimaAssets.townScreen.textureId, townTransform, viewMatrix);

  uiConsole_update(deltaTime);
}

void sceneTown_free() {
  geometry_free(&townGeometry);
}

Scene sceneTown = {
  .scene_init = sceneTown_init,
  .scene_update = sceneTown_update,
  .scene_free = sceneTown_free
};