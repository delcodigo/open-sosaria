#include "guardTown.h"
#include "engine/geometry.h"
#include "scenes/sceneDiskLoader.h"
#include "maths/matrix4.h"
#include "config.h"

GuardTown guardTowns[OS_GUARD_TOWN_COUNT] = { 0 };
static Geometry guardTownGeometry;
static float transformMatrix[16];

void guardTown_init() {
  guardTowns[0].x = 17; guardTowns[0].y = 20;
  guardTowns[1].x = 22; guardTowns[1].y = 20;
  guardTowns[2].x = 21; guardTowns[2].y = 10;
  guardTowns[3].x =  3; guardTowns[3].y = 11;
  guardTowns[4].x = 37; guardTowns[4].y =  9;
  guardTowns[5].x = 19; guardTowns[5].y =  2;

  float tx1 = (1.0f * OS_TOWN_CASTLE_SPRITE_WIDTH) / (float)ultimaAssets.townCastleSprites.width;
  float tx2 = (2.0f * OS_TOWN_CASTLE_SPRITE_WIDTH) / (float)ultimaAssets.townCastleSprites.width;
  geometry_setSprite(&guardTownGeometry, OS_TOWN_CASTLE_SPRITE_WIDTH, OS_TOWN_CASTLE_SPRITE_HEIGHT, tx1, 0.0f, tx2, 1.0f);

  matrix4_setIdentity(transformMatrix);
}

void guardTown_render(float *viewMatrix) {
  for (int i=0; i<OS_GUARD_TOWN_COUNT; i++) {
    matrix4_setPosition(transformMatrix, guardTowns[i].x * OS_TOWN_CASTLE_SPRITE_WIDTH, guardTowns[i].y * OS_TOWN_CASTLE_SPRITE_HEIGHT, 2.0f);

    geometry_render(&guardTownGeometry, ultimaAssets.townCastleSprites.textureId, transformMatrix, viewMatrix);
  }
}

void guardTown_free() {
  geometry_free(&guardTownGeometry);
}