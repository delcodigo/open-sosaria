#ifndef OS_UI_CONSOLE_H
#define OS_UI_CONSOLE_H

#include "engine/geometry.h"
#include "engine/text.h"

extern int queuedMessagesCount;

typedef struct {
  Text text;
  char line[40];
  bool isFlashing;
  bool isTypewriter;
} ConsoleLine;

void uiConsole_init();
void uiConsole_setSpaceLabels();
void uiConsole_queueMessage(const char *message);
void uiConsole_queueMessageFormat(const char *format, ...);
void uiConsole_addMessage(const char *message);
void uiConsole_addMessageFormat(const char *format, ...);
void uiConsole_replaceLastMessage(const char *message);
void uiConsole_replaceLastMessageFormat(const char *format, ...);
void uiConsole_updateStats();
void uiConsole_updateSpaceStats();
void uiConsole_clearQueue();
void uiConsole_clearConsole();
void uiConsole_renderConsoleOnly();
void uiConsole_update(float deltaTime, bool renderStats);
void uiConsole_free();

#endif