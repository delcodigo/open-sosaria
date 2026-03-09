#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "maths/matrix4.h"
#include "engine/engine.h"
#include "engine/text.h"
#include "engine/scene.h"
#include "memory.h"
#include "scenes/sceneDiskLoader.h"
#include <GLFW/glfw3.h>

static Scene *scene = NULL;

int main(void)
{
  if (!engine_init()) {
    return EXIT_FAILURE;
  }

  if (!engine_loadFont()) {
    engine_cleanup();
    return EXIT_FAILURE;
  }

  scene = &sceneDiskLoader;
  scene->scene_init();

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    scene->scene_update();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  scene->scene_free();
  engine_cleanup();

  printLeftPointers();

  return EXIT_SUCCESS;
}
