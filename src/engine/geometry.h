#ifndef OS_GEOMETRY_H
#define OS_GEOMETRY_H

#include "engine/engine.h"

typedef struct {
  GLuint VAO;
  GLuint VBO;
  GLuint EBO;
  unsigned int indexCount;
} Geometry;

void geometry_setSprite(Geometry *geometry, float width, float height);
void geometry_render(const Geometry *geometry, GLuint textureId, float *transformMatrix, float *viewMatrix);
void geometry_free(Geometry *geometry);

#endif