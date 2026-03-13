#ifndef OS_ENGINE_INPUT_H
#define OS_ENGINE_INPUT_H

#include <stdbool.h>
#include <GLFW/glfw3.h>

typedef struct {
  bool active;
  char text[41];
  int cursorPosition;
  int maxLength;
  bool isDirty;
  bool isSubmitted;
  bool isNumberOnly;
} Textfield;

typedef struct {
  int up;
  int down;
  int left;
  int right;
  int space;
} SosariaInput;

extern Textfield *inputTextfield;
extern SosariaInput input;

void input_charCallback(GLFWwindow* window, unsigned int codepoint);
void input_keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

#endif