#include "engine/geometry.h"
#include "engine/camera.h"
#include "maths/matrix4.h"
#include "sceneOverworld.h"
#include "sceneDiskLoader.h"
#include "entities/worldMap.h"
#include "engine/input.h"

static void sceneOverworld_init() {
  worldMap_init();
}

float position[2] = {0.0f, 0.0f};
static void sceneOverworld_updateFlyingCamera() {
  float speed = 4.0f;
  if (input.up) {
    position[1] -= speed;
  }
  if (input.down) {
    position[1] += speed;
  }
  if (input.left) {
    position[0] -= speed;
  }
  if (input.right) {
    position[0] += speed;
  }

  camera_setPosition3f(&camera, position[0], -position[1], 10);
}

static void sceneOverworld_update(float deltaTime) {
  (void) deltaTime;

  sceneOverworld_updateFlyingCamera();
  float *viewMatrix = camera_getViewProjectionMatrix(&camera);
  
  worldMap_update(viewMatrix);
}

static void sceneOverworld_free() {
  worldMap_free();
}

Scene sceneOverworld = {
  .scene_init = sceneOverworld_init,
  .scene_update = sceneOverworld_update,
  .scene_free = sceneOverworld_free
};