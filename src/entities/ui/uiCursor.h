#ifndef OS_UI_CURSOR_H
#define OS_UI_CURSOR_H

#include <stdbool.h>
#include "engine/text.h"

void uiCursor_init();
void uiCursor_update(float deltaTime, float x, float y);
void uiCursor_free();

#endif