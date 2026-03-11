#include "input.h"

Textfield *inputTextfield = NULL;

void input_charCallback(GLFWwindow* window, unsigned int codepoint) {
  (void) window;
  if (inputTextfield == NULL || !inputTextfield->active) {
    return;
  }

  if (codepoint < 32 || codepoint > 126) {
    return;
  }

  if (inputTextfield->isNumberOnly && (codepoint < '0' || codepoint > '9')) {
    return;
  }

  if (inputTextfield->cursorPosition + 1 >= inputTextfield->maxLength) {
    return;
  }

  inputTextfield->text[inputTextfield->cursorPosition++] = (char) codepoint;
  inputTextfield->text[inputTextfield->cursorPosition] = '\0';
  inputTextfield->isDirty = true;
}

void input_keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  (void) window;
  (void) scancode;
  (void) mods;

  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    if (key == GLFW_KEY_BACKSPACE) {
      if (inputTextfield != NULL && inputTextfield->active && inputTextfield->cursorPosition > 0) {
        inputTextfield->text[--inputTextfield->cursorPosition] = '\0';
        inputTextfield->isDirty = true;
      }
    } else if (key == GLFW_KEY_ENTER) {
      if (inputTextfield != NULL && inputTextfield->active) {
        inputTextfield->isDirty = true;
        inputTextfield->isSubmitted = true;
      }
    }
  }
}