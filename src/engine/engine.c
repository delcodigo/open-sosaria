#include <stdio.h>
#include <stdlib.h>
#include "engine.h"
#include "shader.h"
#include "texture.h"
#include "config.h"
#include "memory.h"
#include "input.h"

GLFWwindow *window = NULL;
unsigned int shaderProgram = 0;
GLuint fontTexture = 0;
Camera camera;

static void engine_screenResizeCallback(GLFWwindow *window, int width, int height) {
  (void) window;
  int scaleX = width / OS_SCREEN_WIDTH;
  int scaleY = height / OS_SCREEN_HEIGHT;

  int scale = scaleX < scaleY ? scaleX : scaleY;
  if (scale < 1){
    scale = 1;
  }

  int viewportWidth = OS_SCREEN_WIDTH * scale;
  int viewportHeight = OS_SCREEN_HEIGHT * scale;

  int offsetX = (width - viewportWidth) / 2;
  int offsetY = (height - viewportHeight) / 2;

  glViewport(offsetX, offsetY, viewportWidth, viewportHeight);
}

int engine_init() {
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return 0;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

  window = glfwCreateWindow(OS_SCREEN_WIDTH, OS_SCREEN_HEIGHT, "Open Sosaria", NULL, NULL);
  if (!window) {
    fprintf(stderr, "Failed to create window\n");
    glfwTerminate();
    return 0;
  }

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, engine_screenResizeCallback);
  glfwSwapInterval(1);
  
  glfwSetCharCallback(window, input_charCallback);
  glfwSetKeyCallback(window, input_keyCallback);

  if (!gladLoadGL()) {
    fprintf(stderr, "Failed to load OpenGL functions\n");
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
  }

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  shaderProgram = shader_create_program();
  glUseProgram(shaderProgram);

  camera_createOrthogonal(&camera, OS_SCREEN_WIDTH, OS_SCREEN_HEIGHT, 0.1f, 100.0f);
  camera_setPosition3f(&camera, 0, 0, 10.0f);

  return 1;
}

int engine_loadFont() {
  FILE *fontFile = fopen("font.bin", "rb");
  if (!fontFile) {
    fprintf(stderr, "Failed to open 'font.bin' file\n");
    return 0;
  }

  unsigned int size = 128 * 128 * 4;
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

  fontTexture = texture_load(128, 128, fontData);
  free((void*)fontData);

  return 1;
}

void engine_cleanup() {
  texture_free(fontTexture);
  glDeleteProgram(shaderProgram);
  glfwDestroyWindow(window);
  glfwTerminate();
}