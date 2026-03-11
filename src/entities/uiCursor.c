#include "uiCursor.h"

static Text cursorTextGeometry;
static float cursorBlinkTime = 0.0f;
static bool cursorVisible = true;

void uiCursor_init() {
  char cursorStr[16] = { 0 };
  cursorStr[0] = 127;
  text_create(&cursorTextGeometry, cursorStr, false);
}

void uiCursor_update(float deltaTime, float x, float y) {
  if (cursorVisible) {
    text_render(&cursorTextGeometry, x, y);
  }

  cursorBlinkTime += deltaTime;
  if (cursorBlinkTime >= 0.33f) {
    cursorBlinkTime = 0.0f;
    cursorVisible = !cursorVisible;
  }
}

void uiCursor_free() {
  text_free(&cursorTextGeometry);
}