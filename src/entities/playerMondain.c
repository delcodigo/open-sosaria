#include "playerMondain.h"
#include "engine/geometry.h"
#include "engine/texture.h"
#include "data/player.h"
#include "scenes/sceneDiskLoader.h"
#include "maths/matrix4.h"

static Geometry playerGeometry;
static float transformMatrix[16];

void playerMondain_init() {
  float tx1 = 0;
  float tx2 = 16.0f / (float) ultimaAssets.mondainSprites.width;
  geometry_setSprite(&playerGeometry, 16, 16, tx1, 0, tx2, 1);

  matrix4_setIdentity(transformMatrix);
}

void playerMondain_render(float *viewMatrix) {
  matrix4_setPosition(transformMatrix, player.px * 15, player.py * 15, 1);
  geometry_render(&playerGeometry, ultimaAssets.mondainSprites.textureId, transformMatrix, viewMatrix);
}

bool playerMondain_update(float deltaTime) {
  (void) deltaTime;

  return false;
}

void playerMondain_free() {
  geometry_free(&playerGeometry);
}