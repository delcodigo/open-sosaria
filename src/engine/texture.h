#ifndef OS_TEXTURE_H
#define OS_TEXTURE_H

#include <GLFW/glfw3.h>

GLuint texture_load(unsigned int width, unsigned int height, const unsigned char* data);
void texture_destroy(GLuint texture);

#endif