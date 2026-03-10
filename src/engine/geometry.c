#include "geometry.h"
#include "config.h"
#include "maths/matrix4.h"

void geometry_setSprite(Geometry *geometry, float width, float height) {
  glGenVertexArrays(1, &geometry->VAO);
  glGenBuffers(1, &geometry->VBO);
  glGenBuffers(1, &geometry->EBO);

  int size = OS_QUAD_VERTEX_SIZE * sizeof(float);
  float vertices[] = {
    0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    width, 0.0f, 0.0f, 1.0f, 1.0f,
    0.0f, height, 0.0f, 0.0f, 0.0f,
    width, height, 0.0f, 1.0f, 0.0f
  };
  unsigned int indices[] = {
    0, 1, 2,
    2, 1, 3
  };

  glBindVertexArray(geometry->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, geometry->VBO);
  glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, OS_QUAD_INDEX_SIZE * sizeof(unsigned int), indices, GL_STATIC_DRAW);
  glBindVertexArray(0);

  geometry->indexCount = OS_QUAD_INDEX_SIZE;
}

void geometry_render(const Geometry *geometry, GLuint textureId, float *transformMatrix, float *viewMatrix) {
  if (!geometry) { return; }
  
  glBindVertexArray(geometry->VAO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->EBO);

  glBindTexture(GL_TEXTURE_2D, textureId);
  glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);

  matrix4_setIdentity(transformMatrix);
  
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uModel"), 1, GL_FALSE, transformMatrix);
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uViewProjection"), 1, GL_FALSE, viewMatrix);

  glDrawElements(GL_TRIANGLES, geometry->indexCount, GL_UNSIGNED_INT, 0);
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