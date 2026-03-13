#include "worldMap.h"
#include "config.h"
#include "scenes/sceneDiskLoader.h"
#include "maths/matrix4.h"

Geometry worldMapGeometry = {0};
static float transformMatrix[16] = {0};

void worldMap_init() {
  matrix4_setIdentity(transformMatrix);

  int mapX = 0;
  int mapY = 0;

  float vertices[OS_BTERRA_MAP_WIDTH * OS_BTERRA_MAP_HEIGHT * OS_BTERRA_COUNT * OS_QUAD_VERTEX_SIZE] = {0};
  unsigned int indices[OS_BTERRA_MAP_WIDTH * OS_BTERRA_MAP_HEIGHT * OS_BTERRA_COUNT * OS_QUAD_INDEX_SIZE] = {0};
  int verticesCount = 0;
  int indicesCount = 0;

  for (int i=0;i<OS_BTERRA_COUNT;i++) {
    for (int y=0;y<OS_BTERRA_MAP_HEIGHT;y++) {
      for (int x=0;x<OS_BTERRA_MAP_WIDTH;x++) {
        uint8_t tile = ultimaAssets.bterraMaps[i][y][x];
        
        float tx1 = (tile * OS_TILE_WIDTH) / (float)ultimaAssets.overworldTiles.width;
        float ty1 = 0;
        float tx2 = tx1 + (OS_TILE_WIDTH / (float)ultimaAssets.overworldTiles.width);
        float ty2 = ty1 + (OS_TILE_HEIGHT / (float)ultimaAssets.overworldTiles.height);

        geometry_addQuad(vertices, verticesCount, indices, indicesCount, mapX + x * OS_TILE_WIDTH, mapY + y * OS_TILE_HEIGHT, OS_TILE_WIDTH, OS_TILE_HEIGHT, tx1, ty1, tx2, ty2);
        verticesCount += 4;
        indicesCount += 6;
      }
    }

    mapX += OS_BTERRA_MAP_WIDTH * OS_TILE_WIDTH;
    if (i == 1) {
      mapX = 0;
      mapY += OS_BTERRA_MAP_HEIGHT * OS_TILE_HEIGHT;
    }
  }

  geometry_build(&worldMapGeometry, vertices, verticesCount, indices, indicesCount);
}

void worldMap_update(float *viewMatrix) {
  geometry_render(&worldMapGeometry, ultimaAssets.overworldTiles.textureId, transformMatrix, viewMatrix);
}

void worldMap_free() {
  geometry_free(&worldMapGeometry);
}