#include "engine/geometry.h"
#include "engine/camera.h"
#include "maths/matrix4.h"
#include "sceneOverworld.h"
#include "sceneDiskLoader.h"
#include "entities/worldMap.h"
#include "entities/playerOverworld.h"
#include "entities/ui/uiConsole.h"
#include "engine/input.h"

bool playerActed = false;

static void sceneOverworld_init() {
  worldMap_init();
  playerOverworld_init();
  uiConsole_init();
  uiConsole_addMessage(ultimaStrings[114]);
  playerActed = false;
}

static void sceneOverworld_update(float deltaTime) {
  if (playerActed) {
    playerActed = false;
    uiConsole_addMessage(ultimaStrings[114]);
  }

  if (playerOverworld_update(deltaTime)) { 
    playerActed = true; 
  }

  float *viewMatrix = camera_getViewProjectionMatrix(&camera);
  
  worldMap_update(viewMatrix);

  uiConsole_update();
}

static void sceneOverworld_free() {
  worldMap_free();
  playerOverworld_free();
  uiConsole_free();
}

Scene sceneOverworld = {
  .scene_init = sceneOverworld_init,
  .scene_update = sceneOverworld_update,
  .scene_free = sceneOverworld_free
};