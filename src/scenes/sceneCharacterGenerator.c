#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sceneCharacterGenerator.h"
#include "engine/text.h"
#include "engine/input.h"
#include "entities/uiCursor.h"
#include "data/player.h"
#include "sceneDiskLoader.h"
#include "config.h"

static Text titleTextGeometry;
static Text pointsLeftTextGeometry;
static Text nameTextGeometry;
static Text satisfactoryTextGeometry;
static Text racesAndTypesTextGeometry;
static Text statsTextGeometry[6];
static Text statsValueTextGeometry[6];
static Text classTypeTextGeometry[2];
static Text raceTextGeometry[4];
static Text typeTextGeometry[4];
static Text pointsLeftNumberTextGeometry;
static int pointsLeft = 0;
static int cursorX = 0;
static int cursorY = 0;

static int step = 0;
static Textfield statTextfield = {
  .active = false,
  .text = {0},
  .cursorPosition = 0,
  .maxLength = 3,
  .isDirty = false,
  .isNumberOnly = true,
  .isSubmitted = false
};
static Text statValueTextGeometry;

static void sceneCharacterGenerator_init() {
  text_create(&titleTextGeometry, ultimaStrings[27], false);
  text_create(&pointsLeftTextGeometry, ultimaStrings[28], false);
  text_create(&nameTextGeometry, ultimaStrings[29], false);
  text_create(&satisfactoryTextGeometry, ultimaStrings[30], false);
  text_create(&racesAndTypesTextGeometry, ultimaStrings[31], false);

  for (int i=0;i<6;i++) {
    text_create(&statsTextGeometry[i], ultimaStrings[86 + i], false);
    text_create(&statsValueTextGeometry[i], (char[3]){' ', ' ', '\0'}, false);
  }

  for (int i=0;i<2;i++) {
    text_create(&classTypeTextGeometry[i], ultimaStrings[92 + i], false);
  }

  for (int i=0;i<4;i++) {
    char line[10] = {0};
    snprintf(line, sizeof(line), "%d-%.*s", i + 1, 7, ultimaStrings[94 + i]);
    text_create(&raceTextGeometry[i], line, false);
  }

  for (int i=0;i<4;i++) {
    char line[11] = {0};
    snprintf(line, sizeof(line), "%d-%.*s", i + 1, 8, ultimaStrings[98 + i]);
    text_create(&typeTextGeometry[i], line, false);
  }

  pointsLeft = 90;
  char pointsLeftStr[3];
  snprintf(pointsLeftStr, sizeof(pointsLeftStr), "%d", pointsLeft);
  text_create(&pointsLeftNumberTextGeometry, pointsLeftStr, false);

  char statValueStr[3] = {' ', ' ', '\0'};
  text_create(&statValueTextGeometry, statValueStr, false);

  uiCursor_init();

  step = 0;
  statTextfield.active = true;
  memset(statTextfield.text, 0, sizeof(statTextfield.text));
  statTextfield.cursorPosition = 0;
  statTextfield.isDirty = false;

  memset(&player, 0, sizeof(player));

  inputTextfield = &statTextfield;

  cursorX = 182;
  cursorY = 145;
}

static void sceneCharacterGenerator_submitStatValue(const char *value) {
  if (step >= 6) { return; }

  statTextfield.isSubmitted = false;
  int intValue = atoi(value);

  if ((intValue < 10 || intValue > 20 || intValue >= pointsLeft) && step < 5) {
    statTextfield.text[0] = '\0';
    statTextfield.cursorPosition = 0;
    statTextfield.isDirty = true;
    return;
  }

  pointsLeft -= intValue;
  char pointsLeftStr[3] = {0};
  snprintf(pointsLeftStr, sizeof(pointsLeftStr), "%d", pointsLeft);
  text_update(&pointsLeftNumberTextGeometry, pointsLeftStr, false);

  if (step >= 0 && step < 6) {
    text_update(&statsValueTextGeometry[step], value, false);

    statTextfield.text[0] = '\0';
    statTextfield.cursorPosition = 0;
    statTextfield.isDirty = true;
    cursorX = 182;
    cursorY -= 8;
    step += 1;

    char statValueStr[3] = {0};
    strncpy(statValueStr, statTextfield.text, sizeof(statValueStr) - 1);
    text_update(&statValueTextGeometry, statValueStr, false);

    switch (step) {
      case 0: player.strength = intValue; break;
      case 1: player.agility = intValue; break;
      case 2: player.stamina = intValue; break;
      case 3: player.charisma = intValue; break;
      case 4: player.wisdom = intValue; break;
      case 5: player.intelligence = intValue; break;
    }
  }

  if (step == 6) {
    cursorX = 133;
    cursorY = 89;
  }
}

static void sceneCharacterGenerator_statsUpdate(float deltaTime) {
  if (step >= 0 && step < 6) {
    text_render(&statValueTextGeometry, 182, cursorY);
  } else if (step >= 6 && step < 9) {
    text_render(&statValueTextGeometry, 133, cursorY);
  }
  uiCursor_update(deltaTime, cursorX, cursorY);

  if (statTextfield.isDirty) {
    statTextfield.isDirty = false;
    char statValueStr[3] = {0};
    strncpy(statValueStr, statTextfield.text, sizeof(statValueStr) - 1);
    text_update(&statValueTextGeometry, statValueStr, false);

    if (step >= 0 && step < 6) {
      cursorX = 182;
    } else if (step >= 6 && step < 9) {
      cursorX = 133;
    }

    cursorX += strlen(statTextfield.text) * OS_FONT_GLYPH_WIDTH;

    if (statTextfield.isSubmitted) {
      sceneCharacterGenerator_submitStatValue(statValueStr);
    }
  }
}

static void sceneCharacterGenerator_update(float deltaTime) {
  (void) deltaTime;

  text_render(&titleTextGeometry, 70, 177);
  text_render(&pointsLeftTextGeometry, 42, 161);
  text_render(&pointsLeftNumberTextGeometry, 224, 161);
  text_render(&nameTextGeometry, 98, 73);
  text_render(&satisfactoryTextGeometry, 70, 57);
  text_render(&racesAndTypesTextGeometry, 77, 41);

  for (int i=0;i<6;i++) {
    text_render(&statsTextGeometry[i], 77, 145 - i * 8);
    text_render(&statsValueTextGeometry[i], 182, 145 - i * 8);
  }

  for (int i=0;i<2;i++) {
    text_render(&classTypeTextGeometry[i], 98, 89 - i * 8);
  }

  for (int i=0;i<4;i++) {
    text_render(&raceTextGeometry[i], 70, 33 - i * 8);
    text_render(&typeTextGeometry[i], 140, 33 - i * 8);
  }

  sceneCharacterGenerator_statsUpdate(deltaTime);
}

static void sceneCharacterGenerator_free() {
  text_free(&titleTextGeometry);
  text_free(&pointsLeftTextGeometry);
  text_free(&pointsLeftNumberTextGeometry);
  text_free(&nameTextGeometry);
  text_free(&satisfactoryTextGeometry);
  text_free(&racesAndTypesTextGeometry);

  for (int i=0;i<6;i++) {
    text_free(&statsTextGeometry[i]);
    text_free(&statsValueTextGeometry[i]);
  }

  for (int i=0;i<2;i++) {
    text_free(&classTypeTextGeometry[i]);
  }

  for (int i=0;i<4;i++) {
    text_free(&raceTextGeometry[i]);
    text_free(&typeTextGeometry[i]);
  }

  text_free(&statValueTextGeometry);

  uiCursor_free();
}

Scene sceneCharacterGenerator = {
  .scene_init = sceneCharacterGenerator_init,
  .scene_update = sceneCharacterGenerator_update,
  .scene_free = sceneCharacterGenerator_free
};