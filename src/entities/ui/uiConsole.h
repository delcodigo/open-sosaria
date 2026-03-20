#ifndef OS_UI_CONSOLE_H
#define OS_UI_CONSOLE_H

#include "engine/geometry.h"

extern int queuedMessagesCount;

void uiConsole_init();
void uiConsole_queueMessage(const char *message);
void uiConsole_addMessage(const char *message);
void uiConsole_replaceLastMessage(const char *message);
void uiConsole_updateStats();
void uiConsole_update(float deltaTime);
void uiConsole_free();

#endif