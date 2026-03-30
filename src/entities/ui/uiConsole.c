#include <stdio.h>
#include <string.h>
#include <stdarg.h>
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
static ConsoleLine consoleLines[4];
static const unsigned char textureData[] = {0,0,0,255};
static GLuint blackPanelTextureId;
static float transformMatrix[16];
static char queuedMessages[20][35] = { 0 };
int queuedMessagesCount = 0;
static float timeToNextMessage = 0.0f;
static bool dequeuing = false;
static float flashTime = 0.0f;
static char flashChar = '0';
static int typewriterIndex = 3;
static float typewriterTime = 0.0f;

void uiConsole_init() {
  geometry_setSprite(&blackPanel, OS_SCREEN_WIDTH, OS_TILE_HEIGHT * 2, 0, 0, 0, 1);
  blackPanelTextureId = texture_load(1, 1, textureData);

  matrix4_setIdentity(transformMatrix);
  matrix4_setPosition(transformMatrix, 0, OS_SCREEN_HEIGHT - OS_TILE_HEIGHT * 2, 2);

  text_create(&statsLabels[0], ultimaStrings[90]);
  text_create(&statsLabels[1], ultimaStrings[92]);
  text_create(&statsLabels[2], ultimaStrings[94]);
  text_create(&statsLabels[3], ultimaStrings[96]);

  char statStr[7] = {0};
  snprintf(statStr, sizeof(statStr), "%d", player.health);
  text_create(&stats[0], "     ");
  snprintf(statStr, sizeof(statStr), "%d", (int)player.food);
  text_create(&stats[1], "     ");
  snprintf(statStr, sizeof(statStr), "%d", player.experience);
  text_create(&stats[2], "     ");
  snprintf(statStr, sizeof(statStr), "%d", player.gold);
  text_create(&stats[3], "     ");

  for (int i=0;i<4;i++) {
    memset(consoleLines[i].line, ' ', sizeof(consoleLines[i].line));
    consoleLines[i].line[29] = '\0';
    consoleLines[i].isFlashing = false;
    text_create(&consoleLines[i].text, consoleLines[i].line);
  }

  uiConsole_updateStats();
}

void uiConsole_queueMessage(const char *message) {
  if (queuedMessagesCount < 20) {
    strncpy(queuedMessages[queuedMessagesCount], message, sizeof(queuedMessages[queuedMessagesCount]));
    queuedMessages[queuedMessagesCount][strlen(message)] = '\0';
    queuedMessagesCount++;
  }
}

void uiConsole_queueMessageFormat(const char *format, ...) {
  char message[35] = {0};
  va_list args;
  va_start(args, format);
  vsnprintf(message, sizeof(message), format, args);
  va_end(args);
  uiConsole_queueMessage(message);
}

static void uiConsole_detectFlashing(int index) {
  int length = strlen(consoleLines[index].line);
  for (int i=0;i<length-1;i++) {
    if (consoleLines[index].line[i] == '^' && consoleLines[index].line[i + 1] == 'F') {
      consoleLines[index].isFlashing = true;
      return;
    }
  }
}

static bool uiConsole_isTypewriter(const char *line) {
  int length = strlen(line);
  for (int i=0;i<length-1;i++) {
    if (line[i] == '^' && line[i + 1] == 'T') {
      return true;
    }
  }

  return false;
}

static void uiConsole_reverseFlashing(int index) {
  int length = strlen(consoleLines[index].line);
  for (int i=0;i<length-3;i++) {
    if (consoleLines[index].line[i] == '^' && consoleLines[index].line[i + 1] == 'F') {
      consoleLines[index].line[i + 3] = flashChar;
      return;
    }
  }
}

void uiConsole_addMessage(const char *message) {
  if (queuedMessagesCount > 0 && !dequeuing) {
    uiConsole_queueMessage(message);
    return;
  }

  for (int i=0;i<3;i++) {
    consoleLines[i].isFlashing = consoleLines[i + 1].isFlashing;
    strncpy(consoleLines[i].line, consoleLines[i + 1].line, sizeof(consoleLines[i].line));
    text_update(&consoleLines[i].text, consoleLines[i].line);
  }

  int length = strlen(message);
  strncpy(consoleLines[3].line, message, sizeof(consoleLines[3].line));
  consoleLines[3].line[length < 29 ? length : 29] = '\0';
  consoleLines[3].isFlashing = false;
  uiConsole_detectFlashing(3);
  text_update(&consoleLines[3].text, consoleLines[3].line);
}

void uiConsole_addMessageFormat(const char *format, ...) {
  char message[35] = {0};
  va_list args;
  va_start(args, format);
  vsnprintf(message, sizeof(message), format, args);
  va_end(args);
  uiConsole_addMessage(message);
}

void uiConsole_replaceLastMessage(const char *message) {
  strncpy(consoleLines[3].line, message, sizeof(consoleLines[3].line));
  consoleLines[3].line[29] = '\0';
  uiConsole_detectFlashing(3);
  text_update(&consoleLines[3].text, consoleLines[3].line);
}

void uiConsole_replaceLastMessageFormat(const char *format, ...) {
  char message[35] = {0};
  va_list args;
  va_start(args, format);
  vsnprintf(message, sizeof(message), format, args);
  va_end(args);
  uiConsole_replaceLastMessage(message);
}

void uiConsole_updateStats() {
  char statStr[7] = {0};
  snprintf(statStr, sizeof(statStr), "%d", player.health);
  text_update(&stats[0], player.health > 99999 ? "*****" : statStr);
  snprintf(statStr, sizeof(statStr), "%d", (int)player.food);
  text_update(&stats[1], (int)player.food > 99999 ? "*****" : statStr);
  snprintf(statStr, sizeof(statStr), "%d", player.experience);
  text_update(&stats[2], player.experience > 99999 ? "*****" : statStr);
  snprintf(statStr, sizeof(statStr), "%d", player.gold);
  text_update(&stats[3], player.gold > 99999 ? "*****" : statStr);
}

static void uiConsole_updateTypewriter(float deltaTime) {
  typewriterTime += deltaTime;
  float speed = 0.15f / (float)(queuedMessages[0][2] - '0');
  if (typewriterTime >= speed) {
    typewriterTime = 0;
    int length = strlen(queuedMessages[0]);
    if (typewriterIndex < length) {
      char currentLine[30] = {0};
      strncpy(currentLine, queuedMessages[0] + 3, typewriterIndex + 1);
      currentLine[typewriterIndex + 1] = '\0';
      if (typewriterIndex <= 3) {
        uiConsole_addMessage(currentLine);
      } else {
        uiConsole_replaceLastMessage(currentLine);
      }
      typewriterIndex++;
    } else {
      typewriterIndex = 3;
      timeToNextMessage = 0.15f;
      for (int i=0;i<queuedMessagesCount - 1;i++) {
        strncpy(queuedMessages[i], queuedMessages[i + 1], sizeof(queuedMessages[i]));
      }
      queuedMessagesCount--;
    }
  }
}

void uiConsole_update(float deltaTime) {
  if (queuedMessagesCount > 0) {
    timeToNextMessage -= deltaTime;
    if (timeToNextMessage <= 0) {
      dequeuing = true;
      
      if (uiConsole_isTypewriter(queuedMessages[0])) {
        uiConsole_updateTypewriter(deltaTime);
      } else {
        uiConsole_addMessage(queuedMessages[0]);
        for (int i=0;i<queuedMessagesCount - 1;i++) {
          strncpy(queuedMessages[i], queuedMessages[i + 1], sizeof(queuedMessages[i]));
        }
        queuedMessagesCount--;
        typewriterTime = 0;
        typewriterIndex = 3;
        timeToNextMessage = 0.15f;
      }
    }
  }

  dequeuing = false;

  float *viewMatrix = camera_getViewProjectionMatrix(&camera);
  matrix4_setPosition(transformMatrix, camera_getX(&camera), camera_getY(&camera) + OS_SCREEN_HEIGHT - OS_TILE_HEIGHT * 2, 2);
  geometry_render(&blackPanel, blackPanelTextureId, transformMatrix, viewMatrix);

  int cx = camera_getX(&camera);
  int cy = camera_getY(&camera) + 160;

  flashTime += deltaTime;
  for (int i=0;i<4;i++) {
    int sy = cy + OS_FONT_GLYPH_HEIGHT * i;

    if (flashTime >= 0.15f && consoleLines[i].isFlashing) {
      uiConsole_reverseFlashing(i);
      text_update(&consoleLines[i].text, consoleLines[i].line);
    }
    
    text_renderxyz(&consoleLines[i].text, cx, sy, 3);
    text_renderxyz(&statsLabels[i], cx + 203, sy, 3);
    text_renderxyz(&stats[i], cx + 238, sy, 3);
  }

  if (flashTime >= 0.15f) {
    flashTime = 0;
    flashChar = flashChar == '0' ? '1' : '0';
  }
}

void uiConsole_free() {
  geometry_free(&blackPanel);
  glDeleteTextures(1, &blackPanelTextureId);

  for (int i=0;i<4;i++) {
    text_free(&statsLabels[i]);
    text_free(&stats[i]);
    text_free(&consoleLines[i].text);
  }
}