#include "scene.h"
#include "input.h"

Scene currentScene = {0};

void scene_load(Scene *newScene) {
  if (currentScene.scene_free) {
    currentScene.scene_free();
  }

  currentScene = *newScene;

  if (inputTextfield != NULL) {
    inputTextfield->active = false;
    inputTextfield = NULL;
  }

  if (currentScene.scene_init) {
    currentScene.scene_init();
  }
}