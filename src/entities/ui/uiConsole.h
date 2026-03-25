#ifndef OS_UI_CONSOLE_H
#define OS_UI_CONSOLE_H

#include "engine/geometry.h"
#include "engine/text.h"

extern int queuedMessagesCount;

typedef struct {
  Text text;
  char line[30];
  bool isFlashing;
  bool isTypewriter;
} ConsoleLine;

void uiConsole_init();
void uiConsole_queueMessage(const char *message);
void uiConsole_queueMessageFormat(const char *format, ...);
void uiConsole_addMessage(const char *message);
void uiConsole_replaceLastMessage(const char *message);
void uiConsole_updateStats();
void uiConsole_update(float deltaTime);
void uiConsole_free();

#endif