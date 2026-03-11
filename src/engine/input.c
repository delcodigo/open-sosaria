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

  if (inputTextfield->cursorPosition + 1 >= inputTextfield->maxLength) {
    return;
  }

  inputTextfield->text[inputTextfield->cursorPosition++] = (char) codepoint;
  inputTextfield->text[inputTextfield->cursorPosition] = '\0';
}