#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "maths/matrix4.h"
#include "engine/engine.h"
#include "memory.h"
#include <GLFW/glfw3.h>
#include "engine/text.h"

int main(void)
{
  if (!engine_init()) {
    return EXIT_FAILURE;
  }

  if (!engine_loadFont()) {
    engine_cleanup();
    return EXIT_FAILURE;
  }

  Text textGeometry;
  text_create(&textGeometry, "--ULTIMA--");

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    text_render(&textGeometry, 105.0f, 92.0f);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  text_free(&textGeometry);
  engine_cleanup();

  printLeftPointers();

  return EXIT_SUCCESS;
}
