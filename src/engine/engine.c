#include <stdio.h>
#include "engine.h"
#include "shader.h"

GLFWwindow *window = NULL;
unsigned int shaderProgram = 0;

int engine_init() {
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return 0;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

  window = glfwCreateWindow(280, 192, "Open Sosaria", NULL, NULL);
  if (!window) {
    fprintf(stderr, "Failed to create window\n");
    glfwTerminate();
    return 0;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  if (!gladLoadGL()) {
    fprintf(stderr, "Failed to load OpenGL functions\n");
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
  }

  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

  shaderProgram = shader_create_program();
  glUseProgram(shaderProgram);

  return 1;
}

void engine_cleanup() {
  glDeleteProgram(shaderProgram);
  glfwDestroyWindow(window);
  glfwTerminate();
}