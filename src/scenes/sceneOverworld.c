#include "sceneOverworld.h"

static void sceneOverworld_init() {
}

static void sceneOverworld_update(float deltaTime) {
  (void) deltaTime;
}

static void sceneOverworld_free() {
}

Scene sceneOverworld = {
  .scene_init = sceneOverworld_init,
  .scene_update = sceneOverworld_update,
  .scene_free = sceneOverworld_free
};