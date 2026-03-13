#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sceneCharacterGenerator.h"
#include "engine/text.h"
#include "engine/input.h"
#include "entities/ui/uiCursor.h"
#include "data/player.h"
#include "data/saveAndLoad.h"
#include "sceneDiskLoader.h"
#include "sceneSplash.h"
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
static Text selectedRaceTextGeometry;
static Text selectedTypeTextGeometry;
static Text nameInputTextGeometry;
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

static int raceBonus[4][6] = {
  {0, 0, 0, 0, 0, 5},
  {0, 5, 0, 0, 0, 0},
  {5, 0, 0, 0, 0, 0},
  {-5, 0, 0, 0, 10, 0}
};

static int typeBonus[4][6] = {
  {10, 10, 0, 0, 0, 0},
  {0, 0, 0, 0, 10, 0},
  {0, 0, 0, 0, 0, 10},
  {0, 10, 0, 0, 0, 0}
};

static void sceneCharacterGenerator_free();

static void sceneCharacterGenerator_init() {
  text_create(&titleTextGeometry, ultimaStrings[27], false);
  text_create(&pointsLeftTextGeometry, ultimaStrings[28], false);
  text_create(&nameTextGeometry, ultimaStrings[29], false);
  text_create(&satisfactoryTextGeometry, ultimaStrings[30], false);
  text_create(&racesAndTypesTextGeometry, ultimaStrings[31], false);
  text_create(&selectedRaceTextGeometry, "       \0", false);
  text_create(&selectedTypeTextGeometry, "       \0", false);
  text_create(&nameInputTextGeometry, "                \0", false);

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
  statTextfield.isNumberOnly = true;
  statTextfield.maxLength = 3;
  statTextfield.isSubmitted = false;
  memset(statTextfield.text, 0, sizeof(statTextfield.text));
  statTextfield.cursorPosition = 0;
  statTextfield.isDirty = false;

  memset(&player, 0, sizeof(player));

  inputTextfield = &statTextfield;

  cursorX = 182;
  cursorY = 40;
}

static void sceneCharacterGenerator_updateStats() {
  for (int i=0;i<6;i++) {
    char statValueStr[3] = {0};
    int statValue = *(&player.strength + i);

    snprintf(statValueStr, sizeof(statValueStr), "%d", statValue);
    text_update(&statsValueTextGeometry[i], statValueStr, false);
  }
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
    cursorY += 8;
    step += 1;

    char statValueStr[3] = {0};
    strncpy(statValueStr, statTextfield.text, sizeof(statValueStr) - 1);
    text_update(&statValueTextGeometry, statValueStr, false);

    *(&player.strength + step - 1) = intValue;
  }

  if (step == 6) {
    sceneCharacterGenerator_updateStats();
    cursorX = 133;
    cursorY = 96;
  }
}

static void sceneCharacterGenerator_setPlayerName(const char *name) {
  char initialChar = name[0];
  
  if ((initialChar < 65 || initialChar > 90) && (initialChar < 97 || initialChar > 122)) {
    statTextfield.text[0] = '\0';
    statTextfield.cursorPosition = 0;
    statTextfield.isDirty = true;
    text_update(&nameInputTextGeometry, "                \0", false);
    return;
  }

  strncpy(player.name, name, sizeof(player.name) - 1);
  text_update(&nameInputTextGeometry, player.name, false);

  cursorX = 203;
  cursorY = 128;
  step = 9;
  statTextfield.text[0] = '\0';
  statTextfield.cursorPosition = 0;
}

static void sceneCharacterGenerator_finishPlayerCreation() {
  player.gold = 100;
  player.food = 100.0f;
  player.health = 100;
  player.tx = 40;
  player.ty = 40;
  player.experience = 1;
  player.time = 0.0f;
  player.transport = 0;
  saveGame();
  scene_load(&sceneSplash);
}

static void sceneCharacterGenerator_statsUpdate(float deltaTime) {
  if (step >= 0 && step < 6) {
    text_renderxyz(&statValueTextGeometry, 182, cursorY, 2);
  } else if (step >= 6 && step < 9) {
    text_renderxyz(&statValueTextGeometry, 133, cursorY, 2);
  }
  uiCursor_update(deltaTime, cursorX, cursorY);

  if (statTextfield.isDirty) {
    if (step >= 0 && step < 6) {
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
    } else if (step >= 6 && step < 8) {
      int value = atoi(statTextfield.text);
      statTextfield.text[0] = '\0';
      statTextfield.cursorPosition = 0;

      if (value >= 1 && value <= 4) {
        if (step == 6) {
          player.race = value;
          text_update(&selectedRaceTextGeometry, ultimaStrings[94 + value - 1], false);
          step += 1;
          cursorY += 8;
          int *statPtr = &player.strength;
          for (int i = 0; i < 6; i++) {
            *statPtr += raceBonus[value - 1][i];
            statPtr++;
          }
          sceneCharacterGenerator_updateStats();
        } else if (step == 7) {
          player.type = value;
          text_update(&selectedTypeTextGeometry, ultimaStrings[98 + value - 1], false);
          step += 1;
          cursorY += 8;
          statTextfield.isNumberOnly = false;
          statTextfield.maxLength = 16;
          int *statPtr = &player.strength;
          for (int i = 0; i < 6; i++) {
            *statPtr += typeBonus[value - 1][i];
            statPtr++;
          }
          sceneCharacterGenerator_updateStats();
        }
      }

      statTextfield.text[0] = '\0';
      statTextfield.cursorPosition = 0;
    } else if (step == 8) {
      strncpy(player.name, statTextfield.text, sizeof(player.name) - 1);
      text_update(&nameInputTextGeometry, player.name, false);
      statTextfield.isDirty = false;
      cursorX = 133 + strlen(player.name) * OS_FONT_GLYPH_WIDTH;

      if (statTextfield.isSubmitted) {
        sceneCharacterGenerator_setPlayerName(statTextfield.text);
        statTextfield.isSubmitted = false;
      }
    } else if (step == 9) {
      if (statTextfield.text[0] == 'y' || statTextfield.text[0] == 'Y') {
        sceneCharacterGenerator_finishPlayerCreation();
      } else {
        sceneCharacterGenerator_free();
        sceneCharacterGenerator_init();
      }
    }
  }
}

static void sceneCharacterGenerator_update(float deltaTime) {
  (void) deltaTime;

  text_render(&titleTextGeometry, 70, 8);
  text_render(&pointsLeftTextGeometry, 42, 24);
  text_render(&pointsLeftNumberTextGeometry, 224, 24);
  text_render(&nameTextGeometry, 98, 112);
  text_render(&satisfactoryTextGeometry, 70, 128);
  text_render(&racesAndTypesTextGeometry, 77, 144);
  text_render(&selectedRaceTextGeometry, 133, 96);
  text_render(&selectedTypeTextGeometry, 133, 104);
  text_render(&nameInputTextGeometry, 133, 112);

  for (int i=0;i<6;i++) {
    text_render(&statsTextGeometry[i], 77, 40 + i * 8);
    text_render(&statsValueTextGeometry[i], 182, 40 + i * 8);
  }

  for (int i=0;i<2;i++) {
    text_render(&classTypeTextGeometry[i], 98, 96 + i * 8);
  }

  for (int i=0;i<4;i++) {
    text_render(&raceTextGeometry[i], 70, 152 + i * 8);
    text_render(&typeTextGeometry[i], 140, 152 + i * 8);
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
  text_free(&selectedRaceTextGeometry);
  text_free(&selectedTypeTextGeometry);
  text_free(&nameInputTextGeometry);

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