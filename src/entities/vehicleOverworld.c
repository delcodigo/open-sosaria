#include "vehicleOverworld.h"
#include "scenes/sceneDiskLoader.h"
#include "engine/camera.h"
#include "maths/matrix4.h"
#include "memory.h"

static Geometry vehiclesGeometries[8];
static float transformMatrix[16];

unsigned char vehiclesMap[OS_BTERRA_MAP_WIDTH * 2][OS_BTERRA_MAP_HEIGHT * 2] = { 0 };

void vehicleOverworld_init() {
  for (int i=0;i<8;i++) {
    float tx1 = i * OS_TILE_WIDTH / (float) ultimaAssets.overworldTiles.width;
    float ty1 = 0.5f;
    float tx2 = tx1 + OS_TILE_WIDTH / (float) ultimaAssets.overworldTiles.width;
    float ty2 = 1.0f;

    geometry_setSprite(&vehiclesGeometries[i], OS_TILE_WIDTH, OS_TILE_HEIGHT, tx1, ty1, tx2, ty2);
  }
}

void vehicleOverworld_free() {
  for (int i=0;i<8;i++) {
    geometry_free(&vehiclesGeometries[i]);
  }
}

void vehicleOverworld_render(float *viewMatrix) {
  int cx = (int)(camera_getX(&camera) / OS_TILE_WIDTH + OS_BTERRA_MAP_WIDTH * 2) % (OS_BTERRA_MAP_WIDTH * 2);
  int cy = (int)(camera_getY(&camera) / OS_TILE_HEIGHT + OS_BTERRA_MAP_HEIGHT * 2) % (OS_BTERRA_MAP_HEIGHT * 2);

  for (int y=cy;y<OS_SCREEN_HEIGHT / OS_TILE_HEIGHT + cy + 1;y++) {
    for (int x=cx;x<OS_SCREEN_WIDTH / OS_TILE_WIDTH + cx + 1;x++) {
      int tx = x % (OS_BTERRA_MAP_WIDTH * 2);
      int ty = y % (OS_BTERRA_MAP_HEIGHT * 2);
      int vehicleId = vehiclesMap[ty][tx];
      if (vehicleId > 0) {
        matrix4_setIdentity(transformMatrix);
        matrix4_setPosition(transformMatrix, tx * OS_TILE_WIDTH, ty * OS_TILE_HEIGHT, 1.0f);
        
        geometry_render(&vehiclesGeometries[vehicleId], ultimaAssets.overworldTiles.textureId, transformMatrix, viewMatrix);
      }
    }
  }
}
