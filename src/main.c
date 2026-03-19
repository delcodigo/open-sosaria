#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "maths/matrix4.h"
#include "engine/engine.h"
#include "engine/text.h"
#include "engine/scene.h"
#include "memory.h"
#include "scenes/sceneDiskLoader.h"
#include "data/enemy.h"
#include "data/bevery.h"
#include <GLFW/glfw3.h>

int main(void)
{
  if (!engine_init()) {
    return EXIT_FAILURE;
  }

  if (!engine_loadFont()) {
    engine_cleanup();
    return EXIT_FAILURE;
  }

  scene_load(&sceneDiskLoader);

  srand((unsigned int)time(NULL));

  float lastTime = 0.0;
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    
    float currentTime = (float) glfwGetTime();
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (currentScene.scene_update) {
      currentScene.scene_update(deltaTime);
    }

    glfwSwapBuffers(window);
  }

  if (currentScene.scene_free) {
    currentScene.scene_free();
  }

  engine_cleanup();
  sceneDiskLoader_freeTextures();
  enemy_freeDefinitions();
  bevery_free();

  printLeftPointers();

  return EXIT_SUCCESS;
}
