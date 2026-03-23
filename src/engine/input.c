#include "engine.h"
#include "input.h"
#include <string.h>

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

  if (action == GLFW_PRESS && inputTextfield != NULL && inputTextfield->active && inputTextfield->isAnyKey) {
    inputTextfield->isDirty = true;
    inputTextfield->isSubmitted = true;
    inputTextfield->lastKey = key;
  }

  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    if (key == GLFW_KEY_BACKSPACE) {
      if (inputTextfield != NULL && inputTextfield->active && inputTextfield->cursorPosition > 0) {
        inputTextfield->text[--inputTextfield->cursorPosition] = '\0';
        inputTextfield->isDirty = true;
        inputTextfield->lastKey = key;
      }
    } else if (key == GLFW_KEY_ENTER) {
      if (inputTextfield != NULL && inputTextfield->active) {
        inputTextfield->isDirty = true;
        inputTextfield->isSubmitted = true;
        inputTextfield->lastKey = key;
      }
    }
  }

  switch (key) {
    case GLFW_KEY_UP:
      if (action == GLFW_RELEASE) {
        input.up = 0;
      } else if (input.up == 0 && action == GLFW_PRESS) {
        input.up = 1;
      }
      break;
    case GLFW_KEY_DOWN:
      if (action == GLFW_RELEASE) {
        input.down = 0;
      } else if (input.down == 0 && action == GLFW_PRESS) {
        input.down = 1;
      }
      break;
    case GLFW_KEY_LEFT:
      if (action == GLFW_RELEASE) {
        input.left = 0;
      } else if (input.left == 0 && action == GLFW_PRESS) {
        input.left = 1;
      }
      break;
    case GLFW_KEY_RIGHT:
      if (action == GLFW_RELEASE) {
        input.right = 0;
      } else if (input.right == 0 && action == GLFW_PRESS) {
        input.right = 1;
      }
      break;
    case GLFW_KEY_SPACE:
      if (action == GLFW_RELEASE) {
        input.space = 0;
      } else if (input.space == 0 && action == GLFW_PRESS) {
        input.space = 1;
      }
      break;
    case GLFW_KEY_Z:
      if (action == GLFW_RELEASE) {
        input.z = 0;
      } else if (input.z == 0 && action == GLFW_PRESS) {
        input.z = 1;
      }
      break;
    case GLFW_KEY_Q:
      if (action == GLFW_RELEASE) {
        input.q = 0;
      } else if (input.q == 0 && action == GLFW_PRESS) {
        input.q = 1;
      }
      break;
    case GLFW_KEY_I:
      if (action == GLFW_RELEASE) {
        input.i = 0;
      } else if (input.i == 0 && action == GLFW_PRESS) {
        input.i = 1;
      }
      break;
    case GLFW_KEY_G:
      if (action == GLFW_RELEASE) {
        input.g = 0;
      } else if (input.g == 0 && action == GLFW_PRESS) {
        input.g = 1;
      }
      break;
    case GLFW_KEY_O:
      if (action == GLFW_RELEASE) {
        input.o = 0;
      } else if (input.o == 0 && action == GLFW_PRESS) {
        input.o = 1;
      }
      break;
    case GLFW_KEY_D:
      if (action == GLFW_RELEASE) {
        input.d = 0;
      } else if (input.d == 0 && action == GLFW_PRESS) {
        input.d = 1;
      }
      break;
    case GLFW_KEY_R:
      if (action == GLFW_RELEASE) {
        input.r = 0;
      } else if (input.r == 0 && action == GLFW_PRESS) {
        input.r = 1;
      }
      break;
    case GLFW_KEY_W:
      if (action == GLFW_RELEASE) {
        input.w = 0;
      } else if (input.w == 0 && action == GLFW_PRESS) {
        input.w = 1;
      }
      break;
    case GLFW_KEY_A:
      if (action == GLFW_RELEASE) {
        input.a = 0;
      } else if (input.a == 0 && action == GLFW_PRESS) {
        input.a = 1;
      }
      break;
    case GLFW_KEY_S:
      if (action == GLFW_RELEASE) {
        input.s = 0;
      } else if (input.s == 0 && action == GLFW_PRESS) {
        input.s = 1;
      }
      break;
    case GLFW_KEY_C:
      if (action == GLFW_RELEASE) {
        input.c = 0;
      } else if (input.c == 0 && action == GLFW_PRESS) {
        input.c = 1;
      }
      break;
  }
}

bool input_areKeysReleased() {
  for (int i = 32; i < 128; i++) {
    if (glfwGetKey(window, i) == GLFW_PRESS) {
      return false;
    }
  }

  return true;
}