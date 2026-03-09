#include <stdio.h>
#include <stdlib.h>
#include "engine.h"
#include "shader.h"
#include "texture.h"

GLFWwindow *window = NULL;
unsigned int shaderProgram = 0;
GLuint fontTexture = 0;
Camera camera;

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

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  shaderProgram = shader_create_program();
  glUseProgram(shaderProgram);

  camera_createOrthogonal(&camera, 280, 192, 0.1f, 100.0f);
  camera_setPosition3f(&camera, 140.0f, 96.0f, 10.0f);

  return 1;
}

int engine_loadFont() {
  FILE *fontFile = fopen("font.bin", "rb");
  if (!fontFile) {
    fprintf(stderr, "Failed to open 'font.bin' file\n");
    return 0;
  }

  unsigned int size = 128 * 64 * 4;
  unsigned char *fontData = (unsigned char*) malloc(size);

  if (!fontData) {
    fprintf(stderr, "Failed to allocate memory for font data\n");
    fclose(fontFile);
    return 0;
  }

  unsigned int bytesRead = fread(fontData, 1, size, fontFile);
  fclose(fontFile);

  if (bytesRead != size) {
    fprintf(stderr, "Failed to read font data from 'font.bin'\n");
    free((void*)fontData);
    return 0;
  }

  fontTexture = texture_load(128, 64, fontData);
  free((void*)fontData);

  return 1;
}

void engine_cleanup() {
  texture_destroy(fontTexture);
  glDeleteProgram(shaderProgram);
  glfwDestroyWindow(window);
  glfwTerminate();
}