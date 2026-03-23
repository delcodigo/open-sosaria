#include <string.h>
#include <stdlib.h>
#include "text.h"
#include "memory.h"
#include "config.h"
#include "maths/matrix4.h"

static float transformMatrix[16];

static bool text_isFormattingCode(const char *text) {
  return text[0] == '^' && (text[1] == '0' || text[1] == '1' || text[1] == 'F' || text[1] == 'T');
}

static unsigned int text_getVisibleLength(const char *text) {
  unsigned int visibleLength = 0;

  for (int i=0;text[i] != '\0';i++) {
    if (text_isFormattingCode(&text[i])) {
      i++;
      continue;
    }

    visibleLength++;
  }

  return visibleLength;
}

static void text_addGlyph(char glyph, unsigned int glyphIndex, float *vertices, unsigned int *indices, int *vertexOffset, int *indexOffset, float *x1, bool isInverted) {
  unsigned char character = (unsigned char) glyph;
  int c = character - OS_FONT_OFFSET;
  int row = c / (OS_FONT_WIDTH / OS_FONT_GLYPH_WIDTH);
  int col = c % (OS_FONT_WIDTH / OS_FONT_GLYPH_WIDTH);

  float yOff = isInverted ? 0.5f : 0.0f;
  float x2 = *x1 + OS_FONT_GLYPH_WIDTH;
  float y2 = OS_FONT_GLYPH_HEIGHT;
  float tx1 = (float)(col * OS_FONT_GLYPH_WIDTH) / OS_FONT_WIDTH;
  float ty1 = (float)(row * OS_FONT_GLYPH_HEIGHT) / OS_FONT_HEIGHT + yOff;
  float tx2 = (float)((col + 1) * OS_FONT_GLYPH_WIDTH) / OS_FONT_WIDTH;
  float ty2 = (float)((row + 1) * OS_FONT_GLYPH_HEIGHT) / OS_FONT_HEIGHT + yOff;

  vertices[(*vertexOffset)++] = *x1;
  vertices[(*vertexOffset)++] = y2;
  vertices[(*vertexOffset)++] = 0.0f;
  vertices[(*vertexOffset)++] = tx1;
  vertices[(*vertexOffset)++] = ty2;

  vertices[(*vertexOffset)++] = x2;
  vertices[(*vertexOffset)++] = y2;
  vertices[(*vertexOffset)++] = 0.0f;
  vertices[(*vertexOffset)++] = tx2;
  vertices[(*vertexOffset)++] = ty2;

  vertices[(*vertexOffset)++] = *x1;
  vertices[(*vertexOffset)++] = 0.0f;
  vertices[(*vertexOffset)++] = 0.0f;
  vertices[(*vertexOffset)++] = tx1;
  vertices[(*vertexOffset)++] = ty1;

  vertices[(*vertexOffset)++] = x2;
  vertices[(*vertexOffset)++] = 0.0f;
  vertices[(*vertexOffset)++] = 0.0f;
  vertices[(*vertexOffset)++] = tx2;
  vertices[(*vertexOffset)++] = ty1;

  indices[(*indexOffset)++] = glyphIndex * 4 + 0;
  indices[(*indexOffset)++] = glyphIndex * 4 + 1;
  indices[(*indexOffset)++] = glyphIndex * 4 + 2;
  indices[(*indexOffset)++] = glyphIndex * 4 + 1;
  indices[(*indexOffset)++] = glyphIndex * 4 + 3;
  indices[(*indexOffset)++] = glyphIndex * 4 + 2;

  *x1 = x2;
}

static bool text_addGlyphs(const char *text, float *vertices, unsigned int *indices, unsigned int glyphCount) {
  int vertexOffset = 0;
  int indexOffset = 0;
  float x1 = 0;
  bool isInverted = false;
  unsigned int glyphIndex = 0;

  for (int i=0;text[i] != '\0' && glyphIndex < glyphCount;i++) {
    if (text_isFormattingCode(&text[i])) {
      if (text[i + 1] != 'F'){
        isInverted = text[i + 1] == '1';
      }
      
      i++;
      continue;
    }

    text_addGlyph(text[i], glyphIndex++, vertices, indices, &vertexOffset, &indexOffset, &x1, isInverted);
  }

  while (glyphIndex < glyphCount) {
    text_addGlyph(' ', glyphIndex++, vertices, indices, &vertexOffset, &indexOffset, &x1, isInverted);
  }

  return isInverted;
}

void text_update(Text *textGeometry, const char* text) {
  if (!textGeometry || !textGeometry->vertices || !textGeometry->indices || !text) { return; }

  memset(textGeometry->vertices, 0, textGeometry->size);
  memset(textGeometry->indices, 0, textGeometry->length * OS_QUAD_INDEX_SIZE * sizeof(unsigned int));

  if (textGeometry->length > 0) {
    textGeometry->isInverted = text_addGlyphs(text, textGeometry->vertices, textGeometry->indices, textGeometry->length);
  }

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

  textGeometry->length = text_getVisibleLength(text);
  textGeometry->size = textGeometry->length * OS_QUAD_VERTEX_SIZE * sizeof(float);
  textGeometry->vertices = (float*) malloc(textGeometry->size);
  textGeometry->indices = (unsigned int*) malloc(textGeometry->length * OS_QUAD_INDEX_SIZE * sizeof(unsigned int));

  textGeometry->isInverted = text_addGlyphs(text, textGeometry->vertices, textGeometry->indices, textGeometry->length);

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

void text_renderxyz(Text *textGeometry, float x, float y, float z) {
  Geometry *geometry = &textGeometry->geometry;
  glBindVertexArray(geometry->VAO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->EBO);

  glBindTexture(GL_TEXTURE_2D, fontTexture);
  glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);

  matrix4_setIdentity(transformMatrix);
  matrix4_setPosition(transformMatrix, x, y, z);
  
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uModel"), 1, GL_FALSE, transformMatrix);
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uViewProjection"), 1, GL_FALSE, camera_getViewProjectionMatrix(&camera));

  glDrawElements(GL_TRIANGLES, geometry->indexCount, GL_UNSIGNED_INT, 0);
}

void text_render(Text *textGeometry, float x, float y) {
  text_renderxyz(textGeometry, x, y, 0);
}

void text_free(Text *textGeometry) {
  if (!textGeometry || !textGeometry->vertices) { return; }

  geometry_free(&textGeometry->geometry);
  free(textGeometry->vertices);
  free(textGeometry->indices);

  textGeometry->vertices = NULL;
  textGeometry->indices = NULL;
  textGeometry->length = 0;
  textGeometry->size = 0;
}