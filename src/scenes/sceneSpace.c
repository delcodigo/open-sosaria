#include "sceneSpace.h"
#include "entities/ui/uiConsole.h"

static void sceneSpace_init() {

}

static void sceneSpace_update(float deltaTime) {
  uiConsole_update(deltaTime);
}

static void sceneSpace_free() {

}

Scene sceneSpace = {
  .scene_init = sceneSpace_init,
  .scene_update = sceneSpace_update,
  .scene_free = sceneSpace_free
};