#include "engine/shader.h"
#include "geometry.h"
#include "config.h"
#include "maths/matrix4.h"

void geometry_addQuad(float *vertices, int verticesCount, unsigned int *indices, int indicesCount, float x, float y, float width, float height, float tx1, float ty1, float tx2, float ty2) {
  if (!vertices || !indices) { return; }

  vertices[verticesCount * 5 + 0] = x;
  vertices[verticesCount * 5 + 1] = y;
  vertices[verticesCount * 5 + 2] = 0.0f;
  vertices[verticesCount * 5 + 3] = tx1;
  vertices[verticesCount * 5 + 4] = ty1;

  vertices[(verticesCount + 1) * 5 + 0] = x + width;
  vertices[(verticesCount + 1) * 5 + 1] = y;
  vertices[(verticesCount + 1) * 5 + 2] = 0.0f;
  vertices[(verticesCount + 1) * 5 + 3] = tx2;
  vertices[(verticesCount + 1) * 5 + 4] = ty1;

  vertices[(verticesCount + 2) * 5 + 0] = x;
  vertices[(verticesCount + 2) * 5 + 1] = y - height;
  vertices[(verticesCount + 2) * 5 + 2] = 0.0f;
  vertices[(verticesCount + 2) * 5 + 3] = tx1;
  vertices[(verticesCount + 2) * 5 + 4] = ty2;

  vertices[(verticesCount + 3) * 5 + 0] = x + width;
  vertices[(verticesCount + 3) * 5 + 1] = y - height;
  vertices[(verticesCount + 3) * 5 + 2] = 0.0f;
  vertices[(verticesCount + 3) * 5 + 3] = tx2;
  vertices[(verticesCount + 3) * 5 + 4] = ty2;

  indices[indicesCount + 0] = verticesCount;
  indices[indicesCount + 1] = verticesCount + 1;
  indices[indicesCount + 2] = verticesCount + 2;
  indices[indicesCount + 3] = verticesCount + 2;
  indices[indicesCount + 4] = verticesCount + 1;
  indices[indicesCount + 5] = verticesCount + 3;
}

void geometry_build(Geometry *geometry, const float *vertices, int verticesCount, const unsigned int *indices, int indicesCount) {
  if (!geometry || !vertices || !indices) { return; }

  glGenVertexArrays(1, &geometry->VAO);
  glGenBuffers(1, &geometry->VBO);
  glGenBuffers(1, &geometry->EBO);

  glBindVertexArray(geometry->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, geometry->VBO);
  glBufferData(GL_ARRAY_BUFFER, verticesCount * 5 * sizeof(float), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);
  glBindVertexArray(0);

  geometry->indexCount = indicesCount;
}

void geometry_setSprite(Geometry *geometry, float width, float height, float tx1, float ty1, float tx2, float ty2) {
  glGenVertexArrays(1, &geometry->VAO);
  glGenBuffers(1, &geometry->VBO);
  glGenBuffers(1, &geometry->EBO);

  float vertices[] = {
    0.0f, 0.0f, 0.0f, tx1, ty1,
    width, 0.0f, 0.0f, tx2, ty1,
    0.0f, -height, 0.0f, tx1, ty2,
    width, -height, 0.0f, tx2, ty2
  };
  unsigned int indices[] = {
    0, 1, 2,
    2, 1, 3
  };

  geometry_build(geometry, vertices, 4, indices, 6);
}

void geometry_render(const Geometry *geometry, GLuint textureId, float *transformMatrix, float *viewMatrix) {
  if (!geometry) { return; }

  glBindVertexArray(geometry->VAO);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureId);

  transformMatrix[13] = OS_SCREEN_HEIGHT - transformMatrix[13];
  glUniformMatrix4fv(uModelLocation, 1, GL_FALSE, transformMatrix);
  glUniformMatrix4fv(uViewProjectionLocation, 1, GL_FALSE, viewMatrix);

  glDrawElements(GL_TRIANGLES, geometry->indexCount, GL_UNSIGNED_INT, 0);

  transformMatrix[13] = OS_SCREEN_HEIGHT - transformMatrix[13];
}

void geometry_free(Geometry *geometry) {
  if (!geometry) { return; }

  glDeleteVertexArrays(1, &geometry->VAO);
  glDeleteBuffers(1, &geometry->VBO);
  glDeleteBuffers(1, &geometry->EBO);

  geometry->VAO = 0;
  geometry->VBO = 0;
  geometry->EBO = 0;
  geometry->indexCount = 0;
}