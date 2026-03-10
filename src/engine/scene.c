#include "scene.h"

Scene currentScene = {0};

void scene_load(Scene *newScene) {
  if (currentScene.scene_free) {
    currentScene.scene_free();
  }

  currentScene = *newScene;

  if (currentScene.scene_init) {
    currentScene.scene_init();
  }
}