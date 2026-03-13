#include <string.h>
#include "sceneMainMenu.h"
#include "data/saveAndLoad.h"
#include "engine/text.h"
#include "engine/input.h"
#include "entities/ui/uiCursor.h"
#include "sceneDiskLoader.h"
#include "sceneCharacterGenerator.h"
#include "sceneOverworld.h"

static Text titleTextGeometry[4];
static Text copyrightTextGeometry[2];
static Text optionsTextGeometry[2];
static Text choiceTextGeometry;
static Text noSavedGameTextGeometry;
static bool displayNoSavedGameMessage = false;

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

  text_create(&noSavedGameTextGeometry, "NO SAVED GAME FOUND", false);

  uiCursor_init();

  menuInputField.active = true;
  memset(menuInputField.text, 0, sizeof(menuInputField.text));
  menuInputField.cursorPosition = 0;
  inputTextfield = &menuInputField;
}

static void sceneMainMenu_resetInputText(bool isNumberOnly) {
  menuInputField.isDirty = false;
  menuInputField.cursorPosition = 0;
  menuInputField.isNumberOnly = isNumberOnly;
  memset(menuInputField.text, 0, sizeof(menuInputField.text));

}

static void sceneMainMenu_update(float deltaTime) {
  if (displayNoSavedGameMessage) {
    text_render(&noSavedGameTextGeometry, 74, 96);
    if (menuInputField.isDirty) {
      displayNoSavedGameMessage = false;
      sceneMainMenu_resetInputText(true);
    }

    return;
  }

  text_render(&titleTextGeometry[0], 91, 8);
  text_render(&titleTextGeometry[1], 91, 24);
  text_render(&titleTextGeometry[2], 91, 40);
  text_render(&titleTextGeometry[3], 91, 56);

  text_render(&copyrightTextGeometry[0], 0, 80);
  text_render(&copyrightTextGeometry[1], 0, 88);

  text_render(&optionsTextGeometry[0], 0, 128);
  text_render(&optionsTextGeometry[1], 0, 144);

  text_render(&choiceTextGeometry, 119, 168);

  uiCursor_update(deltaTime, 168, 168);

  if (menuInputField.text[0] == '1') {
    menuInputField.active = false;
    scene_load(&sceneCharacterGenerator);
  } else if (menuInputField.text[0] == '2') {
    if (!loadGame()) {
      displayNoSavedGameMessage = true;
      sceneMainMenu_resetInputText(false);
    } else {
      menuInputField.active = false;
      scene_load(&sceneOverworld);
    }
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
  text_free(&noSavedGameTextGeometry);
  uiCursor_free();
}

Scene sceneMainMenu = {
  .scene_init = sceneMainMenu_init,
  .scene_update = sceneMainMenu_update,
  .scene_free = sceneMainMenu_free
};