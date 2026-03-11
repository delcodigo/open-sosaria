#include <string.h>
#include "sceneMainMenu.h"
#include "engine/text.h"
#include "engine/input.h"
#include "entities/uiCursor.h"
#include "sceneDiskLoader.h"
#include "sceneCharacterGenerator.h"

static Text titleTextGeometry[4];
static Text copyrightTextGeometry[2];
static Text optionsTextGeometry[2];
static Text choiceTextGeometry;

static Textfield menuInputField = {
  .active = false,
  .text = {0},
  .cursorPosition = 0,
  .maxLength = 2,
  .isDirty = false,
  .isNumberOnly = true,
  .isSubmitted = false
};

static void sceneMainMenu_init() {
  text_create(&titleTextGeometry[0], ultimaStrings[0], true);
  text_create(&titleTextGeometry[1], ultimaStrings[1], false);
  text_create(&titleTextGeometry[2], ultimaStrings[2], false);
  text_create(&titleTextGeometry[3], ultimaStrings[3], false);

  text_create(&copyrightTextGeometry[0], ultimaStrings[4], false);
  text_create(&copyrightTextGeometry[1], ultimaStrings[5], false);

  text_create(&optionsTextGeometry[0], ultimaStrings[7], false);
  text_create(&optionsTextGeometry[1], ultimaStrings[8], false);

  text_create(&choiceTextGeometry, ultimaStrings[9], false);

  uiCursor_init();

  menuInputField.active = true;
  memset(menuInputField.text, 0, sizeof(menuInputField.text));
  menuInputField.cursorPosition = 0;
  inputTextfield = &menuInputField;
}

static void sceneMainMenu_update(float deltaTime) {
  text_render(&titleTextGeometry[0], 91, 177);
  text_render(&titleTextGeometry[1], 91, 161);
  text_render(&titleTextGeometry[2], 91, 145);
  text_render(&titleTextGeometry[3], 91, 129);

  text_render(&copyrightTextGeometry[0], 0, 105);
  text_render(&copyrightTextGeometry[1], 0, 97);

  text_render(&optionsTextGeometry[0], 0, 57);
  text_render(&optionsTextGeometry[1], 0, 41);

  text_render(&choiceTextGeometry, 119, 16);

  uiCursor_update(deltaTime, 168, 16);

  if (menuInputField.text[0] == '1') {
    menuInputField.active = false;
    scene_load(&sceneCharacterGenerator);
  } else if (menuInputField.text[0] == '2') {
  }
}

static void sceneMainMenu_free() {
  text_free(&titleTextGeometry[0]);
  text_free(&titleTextGeometry[1]);
  text_free(&titleTextGeometry[2]);
  text_free(&titleTextGeometry[3]);

  text_free(&copyrightTextGeometry[0]);
  text_free(&copyrightTextGeometry[1]);

  text_free(&optionsTextGeometry[0]);
  text_free(&optionsTextGeometry[1]);

  text_free(&choiceTextGeometry);
  uiCursor_free();
}

Scene sceneMainMenu = {
  .scene_init = sceneMainMenu_init,
  .scene_update = sceneMainMenu_update,
  .scene_free = sceneMainMenu_free
};