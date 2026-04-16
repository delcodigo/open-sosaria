#ifndef OS_DUNGEON_RENDERER_H
#define OS_DUNGEON_RENDERER_H

#include <stdint.h>

void dungeonRenderer_setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
void dungeonRenderer_drawLine(int x0, int y0, int x1, int y1);
void dungeonRenderer_init();
void dungeonRenderer_clear();
void dungeonRenderer_update();
void dungeonRenderer_render(float *viewMatrix);
void dungeonRenderer_free();

#endif