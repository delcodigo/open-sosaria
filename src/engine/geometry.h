#ifndef OS_GEOMETRY_H
#define OS_GEOMETRY_H

#include <GLFW/glfw3.h>

typedef struct {
  GLuint VAO;
  GLuint VBO;
  GLuint EBO;
  unsigned int indexCount;
} Geometry;

#endif