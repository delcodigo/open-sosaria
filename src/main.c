#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "engine/engine.h"
#include <GLFW/glfw3.h>

int main(void)
{
  if (!engine_init()) {
    return EXIT_FAILURE;
  }

  float vertices[] = {
    -0.5f, -0.5f, 0.0f,
      0.5f, -0.5f, 0.0f,
      0.0f,  0.5f, 0.0f
  };

  float colors[] = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
  };

  unsigned int vao = 0;
  unsigned int vbo[2] = {0, 0};

  glGenVertexArrays(1, &vao);
  glGenBuffers(2, vbo);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteBuffers(2, vbo);
  glDeleteVertexArrays(1, &vao);
  engine_cleanup();

  return EXIT_SUCCESS;
}
