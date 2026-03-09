#ifndef OS_ENGINE_H
#define OS_ENGINE_H

#include "dependencies/glad.h"
#include <GLFW/glfw3.h>
#include "camera.h"

extern GLFWwindow *window;
extern unsigned int shaderProgram;
extern GLuint fontTexture;
extern Camera camera;

int engine_init();
int engine_loadFont();
void engine_cleanup();

#endif