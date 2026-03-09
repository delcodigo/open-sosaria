#ifndef OS_ENGINE_H
#define OS_ENGINE_H

#include "dependencies/glad.h"
#include <GLFW/glfw3.h>

extern GLFWwindow *window;
extern unsigned int shaderProgram;

int engine_init();
void engine_cleanup();

#endif