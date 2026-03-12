#ifndef OS_GEOMETRY_H
#define OS_GEOMETRY_H

#include "engine/engine.h"

typedef struct {
  GLuint VAO;
  GLuint VBO;
  GLuint EBO;
  unsigned int indexCount;
} Geometry;

void geometry_addQuad(float *vertices, int verticesCount, unsigned int *indices, int indicesCount, float x, float y, float width, float height, float tx1, float ty1, float tx2, float ty2);
void geometry_build(Geometry *geometry, const float *vertices, int verticesCount, const unsigned int *indices, int indicesCount);
void geometry_setSprite(Geometry *geometry, float width, float height, float tx1, float ty1, float tx2, float ty2);
void geometry_render(const Geometry *geometry, GLuint textureId, float *transformMatrix, float *viewMatrix);
void geometry_free(Geometry *geometry);

#endif