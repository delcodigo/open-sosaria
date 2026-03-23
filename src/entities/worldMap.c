#include "worldMap.h"
#include "config.h"
#include "scenes/sceneDiskLoader.h"
#include "maths/matrix4.h"
#include "data/player.h"

Geometry worldMapGeometry[OS_BTERRA_COUNT] = {0};
static float transformMatrix[16] = {0};

void worldMap_init() {
  matrix4_setIdentity(transformMatrix);

  for (int i=0;i<OS_BTERRA_COUNT;i++) {
    Geometry *geometry = &worldMapGeometry[i];
    float vertices[OS_BTERRA_MAP_WIDTH * OS_BTERRA_MAP_HEIGHT * OS_QUAD_VERTEX_SIZE] = {0};
    unsigned int indices[OS_BTERRA_MAP_WIDTH * OS_BTERRA_MAP_HEIGHT * OS_QUAD_INDEX_SIZE] = {0};
    int verticesCount = 0;
    int indicesCount = 0;

    for (int y=0;y<OS_BTERRA_MAP_HEIGHT;y++) {
      for (int x=0;x<OS_BTERRA_MAP_WIDTH;x++) {
        uint8_t tile = (uint8_t)((ultimaAssets.bterraMaps[i][y][x] >> 4) & 0x0F);
        
        float tx1 = (tile * OS_TILE_WIDTH) / (float)ultimaAssets.overworldTiles.width;
        float ty1 = 0;
        float tx2 = tx1 + (OS_TILE_WIDTH / (float)ultimaAssets.overworldTiles.width);
        float ty2 = ty1 + (OS_TILE_HEIGHT / (float)ultimaAssets.overworldTiles.height);

        geometry_addQuad(vertices, verticesCount, indices, indicesCount, x * OS_TILE_WIDTH, y * OS_TILE_HEIGHT, OS_TILE_WIDTH, OS_TILE_HEIGHT, tx1, ty1, tx2, ty2);
        verticesCount += 4;
        indicesCount += 6;
      }
    }

    geometry_build(geometry, vertices, verticesCount, indices, indicesCount);
  }

}

int worldMap_getTileAt(int tx, int ty) {
  int x = (int)(tx % OS_BTERRA_MAP_WIDTH);
  int y = (int)(ty % OS_BTERRA_MAP_HEIGHT);
  int world = ((int)ty / OS_BTERRA_MAP_HEIGHT) * 2 + ((int)tx / OS_BTERRA_MAP_WIDTH);
  return ultimaAssets.bterraMaps[world][y][x];
}

int worldMap_getPlayerTile() {
  return worldMap_getTileAt(player.tx, player.ty);
}

void worldMap_update(float *viewMatrix) {
  int hsign = 1;
  int vsign = 1;
  
  if (player.tx >= OS_BTERRA_MAP_WIDTH * 3 / 2) { hsign = 2; } else { hsign = 0; }
  if (player.ty >= OS_BTERRA_MAP_HEIGHT * 3 / 2) { vsign = 2; } else { vsign = 0; }
  matrix4_setPosition(transformMatrix, OS_BTERRA_MAP_WIDTH * OS_TILE_WIDTH * hsign, OS_BTERRA_MAP_HEIGHT * OS_TILE_HEIGHT * vsign, 0);
  geometry_render(&worldMapGeometry[0], ultimaAssets.overworldTiles.textureId, transformMatrix, viewMatrix);
  
  if (player.tx < OS_BTERRA_MAP_WIDTH / 2) { hsign = -1; } else { hsign = 1; }
  if (player.ty >= OS_BTERRA_MAP_HEIGHT * 3 / 2) { vsign = 2; } else { vsign = 0; }
  matrix4_setPosition(transformMatrix, OS_BTERRA_MAP_WIDTH * OS_TILE_WIDTH * hsign, OS_BTERRA_MAP_HEIGHT * OS_TILE_HEIGHT * vsign, 0);
  geometry_render(&worldMapGeometry[1], ultimaAssets.overworldTiles.textureId, transformMatrix, viewMatrix);
  
  if (player.tx >= OS_BTERRA_MAP_WIDTH * 3 / 2) { hsign = 2; } else { hsign = 0; }
  if (player.ty < OS_BTERRA_MAP_HEIGHT / 2) { vsign = -1; } else { vsign = 1; }
  matrix4_setPosition(transformMatrix, OS_BTERRA_MAP_WIDTH * OS_TILE_WIDTH * hsign, OS_BTERRA_MAP_HEIGHT * OS_TILE_HEIGHT * vsign, 0);
  geometry_render(&worldMapGeometry[2], ultimaAssets.overworldTiles.textureId, transformMatrix, viewMatrix);

  if (player.tx < OS_BTERRA_MAP_WIDTH / 2) { hsign = -1; } else { hsign = 1; }
  if (player.ty < OS_BTERRA_MAP_HEIGHT / 2) { vsign = -1; } else { vsign = 1; }
  matrix4_setPosition(transformMatrix, OS_BTERRA_MAP_WIDTH * OS_TILE_WIDTH * hsign, OS_BTERRA_MAP_HEIGHT * OS_TILE_HEIGHT * vsign, 0);
  geometry_render(&worldMapGeometry[3], ultimaAssets.overworldTiles.textureId, transformMatrix, viewMatrix);
}

void worldMap_free() {
  for (int i = 0; i < OS_BTERRA_COUNT; i++) {
    geometry_free(&worldMapGeometry[i]);
  }
}