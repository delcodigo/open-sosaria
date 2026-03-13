#include <stdio.h>
#include <string.h>
#include "uiztats.h"
#include "uiCursor.h"
#include "config.h"
#include "scenes/sceneDiskLoader.h"
#include "entities/playerOverworld.h"
#include "maths/matrix4.h"
#include "engine/texture.h"
#include "engine/input.h"

static const unsigned char textureData[4] = {0,0,0,255};
static Geometry backgroundPanel;
static Text titleText;
static Text levelClassRaceText;
static Text statLabels[8];
static Text continueText;
static GLuint backgroundTextureId;
static float transformMatrix[16];
static Textfield continueInput = {0};
static int freezeInput = 0;

bool ztatsActive = false;

void uiZtats_free();

void uiZtats_init() {
  matrix4_setIdentity(transformMatrix);

  geometry_setSprite(&backgroundPanel, OS_SCREEN_WIDTH, OS_TILE_HEIGHT, 0, 0, 1, 1);
  backgroundTextureId = texture_load(1, 1, textureData);

  char titleLine[41] = {0};
  snprintf(titleLine, sizeof(titleLine), "%.10s%.10s%.15s", ultimaStrings[260], ultimaStrings[261], player.name);
  text_create(&titleText, titleLine, false);

  text_create(&continueText, ultimaStrings[269], false);

  memset(titleLine, '\0', sizeof(titleLine));
  char level[12] = {0};
  snprintf(level, sizeof(level), "%d", (int)(player.time / 1000.0f) + 1);
  snprintf(titleLine, sizeof(titleLine), "%.10s%.4s %.10s %.10s", ultimaStrings[262], level, ultimaStrings[96 + player.race - 1], ultimaStrings[100 + player.type - 1]);
  text_create(&levelClassRaceText, titleLine, false);

  for (int i=0;i<8;i++) {
    char statLabel[22] = {0};
    snprintf(statLabel, sizeof(statLabel), "%.15s%d", ultimaStrings[86 + i], *(&player.health + i));
    text_create(&statLabels[i], statLabel, false);
  }

  uiCursor_init();

  inputTextfield = &continueInput;
  inputTextfield->active = true;
  inputTextfield->maxLength = 2;
  inputTextfield->isDirty = false;
  inputTextfield->cursorPosition = 0;
  inputTextfield->isAnyKey = true;

  freezeInput = 10;
}

void uiZtats_update(float deltaTime) {
  (void) deltaTime;
  int cx = camera_getX(&camera);
  int cy = camera_getY(&camera);

  matrix4_setPosition(transformMatrix, cx, cy, 8);

  geometry_render(&backgroundPanel, backgroundTextureId, transformMatrix, camera_getViewProjectionMatrix(&camera));
  text_renderxyz(&titleText, cx, cy, 9);
  text_renderxyz(&levelClassRaceText, cx, cy + 8, 9);

  for (int i=0;i<8;i++) {
    text_renderxyz(&statLabels[i], cx, cy + 24 + i * 8, 9);
  }

  text_renderxyz(&continueText, cx, cy + OS_SCREEN_HEIGHT - OS_FONT_GLYPH_HEIGHT, 9);
  uiCursor_update(deltaTime, cx + 70, cy + OS_SCREEN_HEIGHT - OS_FONT_GLYPH_HEIGHT);

  if (freezeInput > 0) {
    freezeInput--;
    inputTextfield->isDirty = false;
    inputTextfield->text[0] = '\0';
    inputTextfield->cursorPosition = 0;
    return;
  }

  if (inputTextfield->isDirty) {
    ztatsActive = false;
    input.z = input.z == 1 ? 2 : 0;
    uiZtats_free();
    memset(&input, 0, sizeof(input));
  }
}

void uiZtats_free() {
  geometry_free(&backgroundPanel);
  text_free(&titleText);
  text_free(&levelClassRaceText);
  for (int i=0;i<8;i++) {
    text_free(&statLabels[i]);
  }
  text_free(&continueText);
  uiCursor_free();

  if (inputTextfield != NULL) {
    inputTextfield->active = false;
    inputTextfield = NULL;
  }
}