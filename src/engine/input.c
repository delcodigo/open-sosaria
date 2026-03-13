#include "input.h"

Textfield *inputTextfield = NULL;
SosariaInput input = {0};

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

  int isDown = action != GLFW_RELEASE;

  switch (key) {
    case GLFW_KEY_UP:
      input.up = isDown;
      break;
    case GLFW_KEY_DOWN:
      input.down = isDown;
      break;
    case GLFW_KEY_LEFT:
      input.left = isDown;
      break;
    case GLFW_KEY_RIGHT:
      input.right = isDown;
      break;
    case GLFW_KEY_SPACE:
      input.space = isDown;
      break;
  }
}