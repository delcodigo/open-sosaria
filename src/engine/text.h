#ifndef OS_TEXT_H
#define OS_TEXT_H

#include "engine.h"
#include "geometry.h"

typedef struct {
  Geometry geometry;
  float *vertices;
  unsigned int *indices;
  unsigned int length;
  unsigned int size;
}Text;

void text_create(Text *textGeometry, const char* text);
void text_update(Text *textGeometry, const char* text);
void text_render(Text *textGeometry, float x, float y);
void text_free(Text *textGeometry);

#endif