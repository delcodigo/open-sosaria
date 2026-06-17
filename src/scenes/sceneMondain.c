#include "sceneMondain.h"
#include "entities/ui/uiConsole.h"

static void sceneMondain_init() {

}

static void sceneMondain_update(float deltaTime) {
  uiConsole_update(deltaTime);
}

static void sceneMondain_free() {

}

Scene sceneMondain = {
  .scene_init = sceneMondain_init,
  .scene_update = sceneMondain_update,
  .scene_free = sceneMondain_free
};