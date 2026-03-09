#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "maths/matrix4.h"
#include "engine/engine.h"
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

  float vertices[] = {
     0.0f, 0.05f, 0.0f,  0.0f, 1.0f,
     128.0f, 0.0f, 0.0f,  1.0f, 1.0f,
     0.0f,  64.0f, 0.0f,  0.0f, 0.0f,
     128.0f,  64.0f, 0.0f,  1.0f, 0.0f
  };

  unsigned int indices[] = {
    0, 1, 2,
    1, 3, 2
  };

  unsigned int vao = 0;
  unsigned int vbo = 0;
  unsigned int ebo = 0;

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
  
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  glBindVertexArray(0);

  float matrix[16];
  matrix4_setIdentity(matrix);

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uModel"), 1, GL_FALSE, matrix);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uViewProjection"), 1, GL_FALSE, camera_getViewProjectionMatrix(&camera));

    glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ebo);
  glDeleteVertexArrays(1, &vao);
  engine_cleanup();

  return EXIT_SUCCESS;
}
