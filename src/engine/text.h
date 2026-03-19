#ifndef OS_TEXT_H
#define OS_TEXT_H

#include <stdbool.h>
#include "engine.h"
#include "geometry.h"

typedef struct {
  Geometry geometry;
  float *vertices;
  unsigned int *indices;
  unsigned int length;
  unsigned int size;
  bool isInverted;
}Text;

void text_create(Text *textGeometry, const char* text, bool isInverted);
void text_update(Text *textGeometry, const char* text, bool isInverted);
void text_renderxyz(Text *textGeometry, float x, float y, float z);
void text_render(Text *textGeometry, float x, float y);
void text_free(Text *textGeometry);

#endif