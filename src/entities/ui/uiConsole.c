#include <stdio.h>
#include <string.h>
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
static Text consoleText[4];
static char consoleLines[4][30] = { 0 };
static const unsigned char textureData[] = {0,0,0,255};
static GLuint blackPanelTextureId;
static float transformMatrix[16];
static bool inverseText = false;
static char queuedMessages[10][30] = { 0 };
static bool queuedInverted[10] = { 0 };
int queuedMessagesCount = 0;
static float timeToNextMessage = 0.0f;
static bool dequeuing = false;

void uiConsole_init() {
  geometry_setSprite(&blackPanel, OS_SCREEN_WIDTH, OS_TILE_HEIGHT * 2, 0, 0, 0, 1);
  blackPanelTextureId = texture_load(1, 1, textureData);

  matrix4_setIdentity(transformMatrix);
  matrix4_setPosition(transformMatrix, 0, OS_SCREEN_HEIGHT - OS_TILE_HEIGHT * 2, 2);

  text_create(&statsLabels[0], ultimaStrings[90], false);
  text_create(&statsLabels[1], ultimaStrings[92], false);
  text_create(&statsLabels[2], ultimaStrings[94], false);
  text_create(&statsLabels[3], ultimaStrings[96], false);

  char statStr[7] = {0};
  snprintf(statStr, sizeof(statStr), "%d", player.health);
  text_create(&stats[0], player.health > 99999 ? "*****" : statStr, false);
  snprintf(statStr, sizeof(statStr), "%d", (int)player.food);
  text_create(&stats[1], (int)player.food > 99999 ? "*****" : statStr, false);
  snprintf(statStr, sizeof(statStr), "%d", player.experience);
  text_create(&stats[2], player.experience > 99999 ? "*****" : statStr, false);
  snprintf(statStr, sizeof(statStr), "%d", player.gold);
  text_create(&stats[3], player.gold > 99999 ? "*****" : statStr, false);

  for (int i=0;i<4;i++) {
    memset(consoleLines[i], ' ', sizeof(consoleLines[i]));
    consoleLines[i][29] = '\0';
    text_create(&consoleText[i], consoleLines[i], false);
  }
}

void uiConsole_inverseText() {
  inverseText = true;
}

void uiConsole_normalText() {
  inverseText = false;
}

void uiConsole_queueMessage(const char *message) {
  if (queuedMessagesCount < 10) {
    strncpy(queuedMessages[queuedMessagesCount], message, sizeof(queuedMessages[queuedMessagesCount]));
    queuedMessages[queuedMessagesCount][29] = '\0';
    queuedInverted[queuedMessagesCount] = inverseText;
    queuedMessagesCount++;
  }
}

void uiConsole_addMessage(const char *message) {
  if (queuedMessagesCount > 0 && !dequeuing) {
    uiConsole_queueMessage(message);
    return;
  }

  for (int i=0;i<3;i++) {
    strncpy(consoleLines[i], consoleLines[i + 1], sizeof(consoleLines[i]));
    text_update(&consoleText[i], consoleLines[i], consoleText[i+1].isInverted);
  }

  strncpy(consoleLines[3], message, sizeof(consoleLines[3]));
  consoleLines[3][29] = '\0';
  text_update(&consoleText[3], consoleLines[3], inverseText);
}

void uiConsole_replaceLastMessage(const char *message) {
  strncpy(consoleLines[3], message, sizeof(consoleLines[3]));
  consoleLines[3][29] = '\0';
  text_update(&consoleText[3], consoleLines[3], consoleText[3].isInverted);
}

void uiConsole_updateStats() {
  char statStr[7] = {0};
  snprintf(statStr, sizeof(statStr), "%d", player.health);
  text_update(&stats[0], player.health > 99999 ? "*****" : statStr, false);
  snprintf(statStr, sizeof(statStr), "%d", (int)player.food);
  text_update(&stats[1], (int)player.food > 99999 ? "*****" : statStr, false);
  snprintf(statStr, sizeof(statStr), "%d", player.experience);
  text_update(&stats[2], player.experience > 99999 ? "*****" : statStr, false);
  snprintf(statStr, sizeof(statStr), "%d", player.gold);
  text_update(&stats[3], player.gold > 99999 ? "*****" : statStr, false);
}

void uiConsole_update(float deltaTime) {
  if (queuedMessagesCount > 0) {
    timeToNextMessage -= deltaTime;
    if (timeToNextMessage <= 0) {
      inverseText = queuedInverted[0];
      dequeuing = true;
      uiConsole_addMessage(queuedMessages[0]);
      for (int i=0;i<queuedMessagesCount - 1;i++) {
        strncpy(queuedMessages[i], queuedMessages[i + 1], sizeof(queuedMessages[i]));
        queuedInverted[i] = queuedInverted[i + 1];
      }
      queuedMessagesCount--;
      timeToNextMessage = 0.3f;
    }
  }

  dequeuing = false;

  float *viewMatrix = camera_getViewProjectionMatrix(&camera);
  matrix4_setPosition(transformMatrix, camera_getX(&camera), camera_getY(&camera) + OS_SCREEN_HEIGHT - OS_TILE_HEIGHT * 2, 2);
  geometry_render(&blackPanel, blackPanelTextureId, transformMatrix, viewMatrix);

  int cx = camera_getX(&camera);
  int cy = camera_getY(&camera) + 160;
  for (int i=0;i<4;i++) {
    int sy = cy + OS_FONT_GLYPH_HEIGHT * i;
    
    text_renderxyz(&consoleText[i], cx, sy, 3);
    text_renderxyz(&statsLabels[i], cx + 203, sy, 3);
    text_renderxyz(&stats[i], cx + 238, sy, 3);
  }
}

void uiConsole_free() {
  geometry_free(&blackPanel);
  glDeleteTextures(1, &blackPanelTextureId);

  for (int i=0;i<4;i++) {
    text_free(&statsLabels[i]);
    text_free(&stats[i]);
    text_free(&consoleText[i]);
  }
}