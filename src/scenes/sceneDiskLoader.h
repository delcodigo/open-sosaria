#ifndef OS_SCENE_DISK_LOADER_H
#define OS_SCENE_DISK_LOADER_H

#include <stdbool.h>
#include "engine/scene.h"
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

  bool loaded;
} UltimaAssets;

void sceneDiskLoader_freeTextures();

extern Scene sceneDiskLoader;
extern UltimaAssets ultimaAssets;

#endif