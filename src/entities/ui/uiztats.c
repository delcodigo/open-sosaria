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
#include "data/bevery.h"

#define UI_ZTATS_VISIBLE_LINES 19

static const unsigned char textureData[4] = {0,0,0,255};
static Geometry backgroundPanel;
static Text titleText;
static Text levelClassRaceText;
static Text statLabelsText[8];
static Text continueText;
static Text armorText[OS_ARMORS_COUNT];
static Text vehiclesText[OS_VEHICLES_COUNT];
static Text weaponsText[OS_WEAPONS_COUNT];
static Text spellsText[OS_SPELLS_COUNT];
static Text gemsText[OS_GEMS_COUNT];
static GLuint backgroundTextureId;
static float transformMatrix[16];
static Textfield continueInput = {0};
static int freezeInput = 0;
static int scroll = 0;

bool ztatsActive = false;

void uiZtats_free();

static void uiZtats_buildTextGeometries(int itemsStart, int itemsEnd, bool isPadded, char **array, Text *textArray, int *playerItems) {
  int textIndex = 0;
  for (int i=itemsStart;i<=itemsEnd;i++) {
    char label[22] = {0};
    int value = *(playerItems + textIndex);
    if (isPadded) {
      int padLength = 15 - strlen(array[i]);
      snprintf(label, sizeof(label), "%.15s%*s%d", array[i], padLength, "", value);
    } else {
      snprintf(label, sizeof(label), "%.15s%d", array[i], value);
    }

    text_create(&textArray[textIndex++], label, false);
  }
}

static void uiZtats_buildTextGeometriesByUltimaIndex(int itemsCount, bool isPadded, int stringIndex, Text *textArray, int *playerItems) {
  for (int i=0;i<itemsCount;i++) {
    char label[22] = {0};
    if (isPadded) {
      int padLength = 15 - strlen(ultimaStrings[stringIndex + i]);
      snprintf(label, sizeof(label), "%.15s%*s%d", ultimaStrings[stringIndex + i], padLength, "", *(playerItems + i));
    } else {
      snprintf(label, sizeof(label), "%.15s%d", ultimaStrings[stringIndex + i], *(playerItems + i));
    }

    text_create(&textArray[i], label, false);
  }
}

void uiZtats_init() {
  matrix4_setIdentity(transformMatrix);

  geometry_setSprite(&backgroundPanel, OS_SCREEN_WIDTH, OS_TILE_HEIGHT, 0, 0, 1, 1);
  backgroundTextureId = texture_load(1, 1, textureData);

  char titleLine[41] = {0};
  snprintf(titleLine, sizeof(titleLine), "%.10s%.10s%.15s", ultimaStrings[242], ultimaStrings[243], player.name);
  text_create(&titleText, titleLine, false);

  text_create(&continueText, "UP/DOWN: SCROLL. ANY KEY: CONTINUE--", false);

  memset(titleLine, '\0', sizeof(titleLine));
  char level[12] = {0};
  snprintf(level, sizeof(level), "%d", (int)(player.time / 1000.0f) + 1);
  snprintf(titleLine, sizeof(titleLine), "%.10s%.4s %.10s %.10s", ultimaStrings[244], level, racesNames[player.race], typesNames[player.type]);
  text_create(&levelClassRaceText, titleLine, false);

  uiZtats_buildTextGeometries(0, 7, false, statsNames, statLabelsText, &player.health);
  uiZtats_buildTextGeometries(1, OS_ARMORS_COUNT, true, armorNames, armorText, player.armors);
  uiZtats_buildTextGeometries(1, OS_VEHICLES_COUNT, true, vehicleNames, vehiclesText, player.vehicles);
  uiZtats_buildTextGeometries(1, OS_WEAPONS_COUNT, true, weaponNames, weaponsText, player.weapons);
  uiZtats_buildTextGeometries(0, OS_SPELLS_COUNT - 1, true, spellNames, spellsText, player.spells);
  uiZtats_buildTextGeometriesByUltimaIndex(OS_GEMS_COUNT, false, 247, gemsText, player.gems);

  uiCursor_init();

  inputTextfield = &continueInput;
  inputTextfield->active = true;
  inputTextfield->maxLength = 2;
  inputTextfield->isDirty = false;
  inputTextfield->cursorPosition = 0;
  inputTextfield->isAnyKey = true;

  freezeInput = 10;
  scroll = 0;
}

static void uiZtats_renderLabels(int itemsCount, Text *texts, int *playerItems, int cx, int cy, int *maxLeftLines, int *yOffset) {
  for (int i=0;i<itemsCount;i++) {
    if (*(playerItems + i) > 0) {
      *maxLeftLines += 1;
      if (*yOffset < 24 || *yOffset >= OS_SCREEN_HEIGHT - 16) { *yOffset += 8; continue; }
      text_renderxyz(&texts[i], cx, cy + *yOffset, 9);
      *yOffset += 8;
    }
  }
}

void uiZtats_update(float deltaTime) {
  (void) deltaTime;
  int cx = camera_getX(&camera);
  int cy = camera_getY(&camera);

  matrix4_setPosition(transformMatrix, cx, cy, 8);

  geometry_render(&backgroundPanel, backgroundTextureId, transformMatrix, camera_getViewProjectionMatrix(&camera));
  text_renderxyz(&titleText, cx, cy, 9);
  text_renderxyz(&levelClassRaceText, cx, cy + 8, 9);

  int yOffset = 24 - scroll * 8;
  int maxLeftLines = 0;

  uiZtats_renderLabels(8, statLabelsText, &player.health, cx, cy, &maxLeftLines, &yOffset);

  yOffset = 88 - scroll * 8;

  uiZtats_renderLabels(OS_ARMORS_COUNT, armorText, player.armors, cx, cy, &maxLeftLines, &yOffset);
  uiZtats_renderLabels(OS_VEHICLES_COUNT, vehiclesText, player.vehicles, cx, cy, &maxLeftLines, &yOffset);
  uiZtats_renderLabels(OS_GEMS_COUNT, gemsText, player.gems, cx, cy, &maxLeftLines, &yOffset);

  yOffset = 24 - scroll * 8;
  int maxRightLines = 0;

  uiZtats_renderLabels(OS_WEAPONS_COUNT, weaponsText, player.weapons, cx + 147, cy, &maxRightLines, &yOffset);
  uiZtats_renderLabels(OS_SPELLS_COUNT, spellsText, player.spells, cx + 147, cy, &maxRightLines, &yOffset);

  text_renderxyz(&continueText, cx, cy + OS_SCREEN_HEIGHT - OS_FONT_GLYPH_HEIGHT, 9);
  uiCursor_update(deltaTime, cx + 252, cy + OS_SCREEN_HEIGHT - OS_FONT_GLYPH_HEIGHT);

  if (freezeInput > 0) {
    freezeInput--;
    inputTextfield->isDirty = false;
    inputTextfield->text[0] = '\0';
    inputTextfield->cursorPosition = 0;
    return;
  }

  if (inputTextfield->isDirty) {
    int maxLines = maxLeftLines > maxRightLines ? maxLeftLines : maxRightLines;
    if (inputTextfield->lastKey == GLFW_KEY_DOWN || inputTextfield->lastKey == GLFW_KEY_UP) {
      if (maxLines >= UI_ZTATS_VISIBLE_LINES){
        scroll += inputTextfield->lastKey == GLFW_KEY_DOWN ? 1 : -1;
        if (scroll < 0) { scroll = 0; }
        if (scroll > maxLines - UI_ZTATS_VISIBLE_LINES) { scroll = maxLines - UI_ZTATS_VISIBLE_LINES; }
      }
      inputTextfield->isDirty = false;
      return;
    }

    ztatsActive = false;
    input.z = input.z == 1 ? 2 : 0;
    uiZtats_free();
    memset(&input, 0, sizeof(input));
  }
}

static void uiZtats_freeTexts(int itemsCount, Text *items) {
  for (int i=0;i<itemsCount;i++) {
    text_free(&items[i]);
  }
}

void uiZtats_free() {
  geometry_free(&backgroundPanel);
  text_free(&titleText);
  text_free(&levelClassRaceText);
  uiZtats_freeTexts(8, statLabelsText);
  text_free(&continueText);
  uiCursor_free();

  uiZtats_freeTexts(OS_ARMORS_COUNT, armorText);
  uiZtats_freeTexts(OS_VEHICLES_COUNT, vehiclesText);
  uiZtats_freeTexts(OS_WEAPONS_COUNT, weaponsText);
  uiZtats_freeTexts(OS_SPELLS_COUNT, spellsText);
  uiZtats_freeTexts(OS_GEMS_COUNT, gemsText);

  if (inputTextfield != NULL) {
    inputTextfield->active = false;
    inputTextfield = NULL;
  }

  texture_free(backgroundTextureId);
}