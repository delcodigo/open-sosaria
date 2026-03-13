#include <stdio.h>
#include "uiConsole.h"
#include "engine/camera.h"
#include "engine/texture.h"
#include "engine/text.h"
#include "scenes/sceneDiskLoader.h"
#include "data/player.h"
#include "maths/matrix4.h"
#include "config.h"

static Geometry blackPanel;
static Text statsLabels[4];
static Text stats[4];
static const unsigned char textureData[] = {0,0,0,(unsigned char)255};
static GLuint blackPanelTextureId;
static float transformMatrix[16];

void uiConsole_init() {
  geometry_setSprite(&blackPanel, OS_SCREEN_WIDTH, OS_TILE_HEIGHT * 2, 0, 0, 0, 1);
  blackPanelTextureId = texture_load(1, 1, textureData);

  matrix4_setIdentity(transformMatrix);
  matrix4_setPosition(transformMatrix, 0, OS_SCREEN_HEIGHT - OS_TILE_HEIGHT * 2, 2);

  text_create(&statsLabels[0], ultimaStrings[106], false);
  text_create(&statsLabels[1], ultimaStrings[108], false);
  text_create(&statsLabels[2], ultimaStrings[110], false);
  text_create(&statsLabels[3], ultimaStrings[112], false);

  char statStr[7] = {0};
  snprintf(statStr, sizeof(statStr), "%d", player.health);
  text_create(&stats[0], statStr, false);
  snprintf(statStr, sizeof(statStr), "%d", player.food);
  text_create(&stats[1], statStr, false);
  snprintf(statStr, sizeof(statStr), "%d", player.experience);
  text_create(&stats[2], statStr, false);
  snprintf(statStr, sizeof(statStr), "%d", player.gold);
  text_create(&stats[3], statStr, false);
}

void uiConsole_update() {
  float *viewMatrix = camera_getViewProjectionMatrix(&camera);
  matrix4_setPosition(transformMatrix, camera_getX(&camera), camera_getY(&camera) + OS_SCREEN_HEIGHT - OS_TILE_HEIGHT * 2, 2);
  geometry_render(&blackPanel, blackPanelTextureId, transformMatrix, viewMatrix);

  int cx = camera_getX(&camera) + 203;
  int cy = camera_getY(&camera) + 160;
  for (int i=0;i<4;i++) {
    text_renderxyz(&statsLabels[i], cx, cy + OS_FONT_GLYPH_HEIGHT * i, 3);
    text_renderxyz(&stats[i], cx + 35, cy + OS_FONT_GLYPH_HEIGHT * i, 3);
  }
}

void uiConsole_free() {
  geometry_free(&blackPanel);
  glDeleteTextures(1, &blackPanelTextureId);

  for (int i=0;i<4;i++) {
    text_free(&statsLabels[i]);
    text_free(&stats[i]);
  }
}