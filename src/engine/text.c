#include <string.h>
#include "text.h"
#include "memory.h"
#include "config.h"
#include "maths/matrix4.h"

static float transformMatrix[16];

static void text_addGlyphs(const char *text, float *vertices, unsigned int *indices) {
  int length = strlen(text);

  int vertexOffset = 0;
  int indexOffset = 0;

  float x1 = 0;
  float y1 = 0;

  for (int i=0;i<length;i++) {
    char c = text[i] - OS_FONT_OFFSET;
    int row = c / (OS_FONT_WIDTH / OS_FONT_GLYPH_WIDTH);
    int col = c % (OS_FONT_WIDTH / OS_FONT_GLYPH_WIDTH);

    float x2 = x1 + OS_FONT_GLYPH_WIDTH;
    float y2 = y1 - OS_FONT_GLYPH_HEIGHT;
    float tx1 = (float)(col * OS_FONT_GLYPH_WIDTH) / OS_FONT_WIDTH;
    float ty1 = (float)(row * OS_FONT_GLYPH_HEIGHT) / OS_FONT_HEIGHT;
    float tx2 = (float)((col + 1) * OS_FONT_GLYPH_WIDTH) / OS_FONT_WIDTH;
    float ty2 = (float)((row + 1) * OS_FONT_GLYPH_HEIGHT) / OS_FONT_HEIGHT;

    vertices[vertexOffset++] = x1;
    vertices[vertexOffset++] = y1;
    vertices[vertexOffset++] = 0.0f;
    vertices[vertexOffset++] = tx1;
    vertices[vertexOffset++] = ty1;

    vertices[vertexOffset++] = x2;
    vertices[vertexOffset++] = y1;
    vertices[vertexOffset++] = 0.0f;
    vertices[vertexOffset++] = tx2;
    vertices[vertexOffset++] = ty1;

    vertices[vertexOffset++] = x1;
    vertices[vertexOffset++] = y2;
    vertices[vertexOffset++] = 0.0f;
    vertices[vertexOffset++] = tx1;
    vertices[vertexOffset++] = ty2;

    vertices[vertexOffset++] = x2;
    vertices[vertexOffset++] = y2;
    vertices[vertexOffset++] = 0.0f;
    vertices[vertexOffset++] = tx2;
    vertices[vertexOffset++] = ty2;

    indices[indexOffset++] = i * 4 + 0;
    indices[indexOffset++] = i * 4 + 1;
    indices[indexOffset++] = i * 4 + 2;
    indices[indexOffset++] = i * 4 + 1;
    indices[indexOffset++] = i * 4 + 3;
    indices[indexOffset++] = i * 4 + 2;

    x1 += OS_FONT_GLYPH_WIDTH;
  }
}

void text_update(Text *textGeometry, const char* text) {
  memset(textGeometry->vertices, 0, textGeometry->size);
  memset(textGeometry->indices, 0, textGeometry->length * OS_QUAD_INDEX_SIZE * sizeof(unsigned int));

  text_addGlyphs(text, textGeometry->vertices, textGeometry->indices);

  glBindBuffer(GL_ARRAY_BUFFER, textGeometry->geometry.VBO);
  glBufferData(GL_ARRAY_BUFFER, textGeometry->size, textGeometry->vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textGeometry->geometry.EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, textGeometry->length * OS_QUAD_INDEX_SIZE * sizeof(unsigned int), textGeometry->indices, GL_STATIC_DRAW);

  textGeometry->geometry.indexCount = textGeometry->length * OS_QUAD_INDEX_SIZE;
}

void text_create(Text *textGeometry, const char* text) {
  Geometry *geometry = &textGeometry->geometry;

  glGenVertexArrays(1, &geometry->VAO);
  glGenBuffers(1, &geometry->VBO);
  glGenBuffers(1, &geometry->EBO);

  textGeometry->length = strlen(text);
  textGeometry->size = textGeometry->length * OS_QUAD_VERTEX_SIZE * sizeof(float);
  textGeometry->vertices = (float*) malloc(textGeometry->size);
  textGeometry->indices = (unsigned int*) malloc(textGeometry->length * OS_QUAD_INDEX_SIZE * sizeof(unsigned int));

  text_addGlyphs(text, textGeometry->vertices, textGeometry->indices);

  glBindVertexArray(geometry->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, geometry->VBO);
  glBufferData(GL_ARRAY_BUFFER, textGeometry->size, textGeometry->vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, textGeometry->length * OS_QUAD_INDEX_SIZE * sizeof(unsigned int), textGeometry->indices, GL_STATIC_DRAW);
  glBindVertexArray(0);

  geometry->indexCount = textGeometry->length * OS_QUAD_INDEX_SIZE;
}

void text_render(Text *textGeometry, float x, float y) {
  Geometry *geometry = &textGeometry->geometry;
  glBindVertexArray(geometry->VAO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->EBO);

  glBindTexture(GL_TEXTURE_2D, fontTexture);
  glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);

  matrix4_setIdentity(transformMatrix);
  matrix4_setPosition(transformMatrix, x, y, 0);
  
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uModel"), 1, GL_FALSE, transformMatrix);
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uViewProjection"), 1, GL_FALSE, camera_getViewProjectionMatrix(&camera));

  glDrawElements(GL_TRIANGLES, geometry->indexCount, GL_UNSIGNED_INT, 0);
}

void text_free(Text *textGeometry) {
  geometry_free(&textGeometry->geometry);
  free(textGeometry->vertices);
  free(textGeometry->indices);
}