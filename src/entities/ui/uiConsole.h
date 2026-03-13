#ifndef OS_UI_CONSOLE_H
#define OS_UI_CONSOLE_H

#include "engine/geometry.h"

void uiConsole_init();
void uiConsole_addMessage(const char *message);
void uiConsole_update();
void uiConsole_free();

#endif