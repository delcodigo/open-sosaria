#ifndef OS_VM_EXECUTER_H
#define OS_VM_EXECUTER_H

#include <stdbool.h>
#include "engine/scene.h"

typedef enum {
  VM_INSTRUCTION_TYPE_NONE,
  VM_INSTRUCTION_TYPE_WAIT,
  VM_INSTRUCTION_TYPE_CHANGE_SCENE,
  VM_INSTRUCTION_TYPE_ADD_CONSOLE_MESSAGE,
  VM_INSTRUCTION_TYPE_REPLACE_CONSOLE_MESSAGE
} VMInstructionType;

typedef struct {
  VMInstructionType type;
  union {
    struct { float duration; } wait;
    struct { Scene *scene; } changeScene;
    struct { char message[31]; } consoleMessage;
  };
} VMInstruction;

void vmExecuter_init(VMInstruction *instructions, int instructionCount);
bool vmExecuter_update(float deltaTime);
void vmExecuter_createSceneTransition(float waitTime, Scene *nextScene);
void vmExecuter_createWait(float waitTime);
void vmExecuter_queueAndReplaceConsoleMessage(char *message1, char *message2, float waitTime);

#endif