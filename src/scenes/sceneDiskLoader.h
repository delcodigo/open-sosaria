#ifndef OS_SCENE_DISK_LOADER_H
#define OS_SCENE_DISK_LOADER_H

#include <stdbool.h>
#include <stdint.h>
#include "engine/scene.h"
#include "config.h"
#include <GLFW/glfw3.h>

typedef struct {
  unsigned char *data;
  unsigned int width;
  unsigned int height;
  GLuint textureId;
} UltimaImage;

typedef struct {
  // HGR splash screens (280x192)
  UltimaImage titleScreen;
  UltimaImage townScreen;
  UltimaImage castleScreen;

  UltimaImage overworldTiles;
  UltimaImage enemySprites;
  UltimaImage townCastleSprites;
  UltimaImage blackSprite;

  uint8_t bterraMaps[OS_BTERRA_COUNT][OS_BTERRA_MAP_WIDTH][OS_BTERRA_MAP_HEIGHT];

  uint8_t townCollisionMap[OS_TOWN_CASTLE_SIZE_HEIGHT][OS_TOWN_CASTLE_SIZE_WIDTH];
  uint8_t castleCollisionMap[OS_TOWN_CASTLE_SIZE_HEIGHT][OS_TOWN_CASTLE_SIZE_WIDTH];

  bool loaded;
} UltimaAssets;

typedef struct {
  uint16_t count;
  uint16_t *starts;
  uint16_t *ends;
  const uint8_t *data;
  uint32_t data_size;
} ShapeTable;

void sceneDiskLoader_freeTextures();

extern Scene sceneDiskLoader;
extern UltimaAssets ultimaAssets;
extern char ultimaStrings[1000][41];

#endif