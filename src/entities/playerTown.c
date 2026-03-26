#include "playerTown.h"
#include "engine/geometry.h"
#include "scenes/sceneDiskLoader.h"
#include "data/player.h"
#include "maths/matrix4.h"
#include "config.h"

static Geometry playerTownGeometry;
static float transformMatrix[16];

void playerTown_init() {
  float tx1 = (6.0f * OS_TOWN_CASTLE_SPRITE_WIDTH) / (float)ultimaAssets.townCastleSprites.width;
  float tx2 = (7.0f * OS_TOWN_CASTLE_SPRITE_WIDTH) / (float)ultimaAssets.townCastleSprites.width;

  geometry_setSprite(&playerTownGeometry, OS_TOWN_CASTLE_SPRITE_WIDTH, OS_TOWN_CASTLE_SPRITE_HEIGHT, tx1, 0, tx2, 1);
}

void playerTown_update(float deltaTime, float *viewMatrix) {
  (void)deltaTime;

  matrix4_setIdentity(transformMatrix);
  matrix4_setPosition(transformMatrix, player.px * OS_TOWN_CASTLE_SPRITE_WIDTH, player.py * OS_TOWN_CASTLE_SPRITE_HEIGHT, 1);

  geometry_render(&playerTownGeometry, ultimaAssets.townCastleSprites.textureId, transformMatrix, viewMatrix);
}

void playerTown_free() {
  geometry_free(&playerTownGeometry);
}