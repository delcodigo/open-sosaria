#ifndef OS_ENGINE_INPUT_H
#define OS_ENGINE_INPUT_H

#include <stdbool.h>
#include <GLFW/glfw3.h>

typedef struct {
  bool active;
  char text[41];
  int cursorPosition;
  int maxLength;
} Textfield;

extern Textfield *inputTextfield;

void input_charCallback(GLFWwindow* window, unsigned int codepoint);

#endif