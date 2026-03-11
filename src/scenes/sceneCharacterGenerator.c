#include <stdio.h>
#include "sceneCharacterGenerator.h"
#include "engine/text.h"
#include "engine/input.h"
#include "entities/uiCursor.h"
#include "sceneDiskLoader.h"

static Text titleTextGeometry;
static Text pointsLeftTextGeometry;
static Text nameTextGeometry;
static Text satisfactoryTextGeometry;
static Text racesAndTypesTextGeometry;
static Text statsTextGeometry[6];
static Text classTypeTextGeometry[2];
static Text raceTextGeometry[4];
static Text typeTextGeometry[4];
static Text pointsLeftNumberTextGeometry;
static int pointsLeft = 90;

static void sceneCharacterGenerator_init() {
  text_create(&titleTextGeometry, ultimaStrings[27], false);
  text_create(&pointsLeftTextGeometry, ultimaStrings[28], false);
  text_create(&nameTextGeometry, ultimaStrings[29], false);
  text_create(&satisfactoryTextGeometry, ultimaStrings[30], false);
  text_create(&racesAndTypesTextGeometry, ultimaStrings[31], false);

  for (int i=0;i<6;i++) {
    text_create(&statsTextGeometry[i], ultimaStrings[86 + i], false);
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

  char pointsLeftStr[3];
  snprintf(pointsLeftStr, sizeof(pointsLeftStr), "%d", pointsLeft);
  text_create(&pointsLeftNumberTextGeometry, pointsLeftStr, false);

  uiCursor_init();
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
  }

  for (int i=0;i<2;i++) {
    text_render(&classTypeTextGeometry[i], 98, 89 - i * 8);
  }

  for (int i=0;i<4;i++) {
    text_render(&raceTextGeometry[i], 70, 33 - i * 8);
    text_render(&typeTextGeometry[i], 140, 33 - i * 8);
  }

  uiCursor_update(deltaTime, 182, 145);
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
  }

  for (int i=0;i<2;i++) {
    text_free(&classTypeTextGeometry[i]);
  }

  for (int i=0;i<4;i++) {
    text_free(&raceTextGeometry[i]);
    text_free(&typeTextGeometry[i]);
  }

  uiCursor_free();
}

Scene sceneCharacterGenerator = {
  .scene_init = sceneCharacterGenerator_init,
  .scene_update = sceneCharacterGenerator_update,
  .scene_free = sceneCharacterGenerator_free
};