#include <stdlib.h>
#include <string.h>
#include "vmExecuter.h"
#include "engine/scene.h"
#include "entities/ui/uiConsole.h"
#include "memory.h"

static VMInstruction *vmInstructions = NULL;
static int vmInstructionCount = 0;
static int currentInstructionIndex = 0;

void vmExecuter_init(VMInstruction *instructions, int instructionCount) {
  vmInstructions = instructions;
  vmInstructionCount = instructionCount;
  currentInstructionIndex = 0;
}

bool vmExecuter_update(float deltaTime) {
  if (vmInstructions == NULL) { return false; }

  VMInstruction *currentInstruction = &vmInstructions[currentInstructionIndex];

  switch (currentInstruction->type) {
    case VM_INSTRUCTION_TYPE_WAIT:
      currentInstruction->wait.duration -= deltaTime;
      if (currentInstruction->wait.duration <= 0) {
        currentInstructionIndex++;
      }
      break;

    case VM_INSTRUCTION_TYPE_CHANGE_SCENE:
      scene_load(currentInstruction->changeScene.scene);
      currentInstructionIndex++;
      break;
    
    case VM_INSTRUCTION_TYPE_ADD_CONSOLE_MESSAGE:
      uiConsole_queueMessage(currentInstruction->consoleMessage.message);
      currentInstructionIndex++;
      break;
    
    case VM_INSTRUCTION_TYPE_REPLACE_CONSOLE_MESSAGE:
      uiConsole_replaceLastMessage(currentInstruction->consoleMessage.message);
      currentInstructionIndex++;
      break;

    default:
      currentInstructionIndex++;
      break;
  }

  if (currentInstructionIndex >= vmInstructionCount) {
    free(vmInstructions);
    vmInstructions = NULL;
    vmInstructionCount = 0;
    currentInstructionIndex = 0;
  }

  return true;
}

void vmExecuter_createSceneTransition(float waitTime, Scene *nextScene) {
  VMInstruction *instructions = malloc(2 * sizeof(VMInstruction));
  instructions[0].type = VM_INSTRUCTION_TYPE_WAIT;
  instructions[0].wait.duration = waitTime;
  instructions[1].type = VM_INSTRUCTION_TYPE_CHANGE_SCENE;
  instructions[1].changeScene.scene = nextScene;
  vmExecuter_init(instructions, 2);
}

void vmExecuter_createWait(float waitTime) {
  VMInstruction *instructions = malloc(sizeof(VMInstruction));
  instructions[0].type = VM_INSTRUCTION_TYPE_WAIT;
  instructions[0].wait.duration = waitTime;
  vmExecuter_init(instructions, 1);
}

void vmExecuter_queueAndReplaceConsoleMessage(char *message1, char *message2, float waitTime) {
  VMInstruction *instructions = malloc(3 * sizeof(VMInstruction));
  instructions[0].type = VM_INSTRUCTION_TYPE_ADD_CONSOLE_MESSAGE;
  memset(instructions[0].consoleMessage.message, 0, sizeof(instructions[0].consoleMessage.message));
  strcpy(instructions[0].consoleMessage.message, message1);
  instructions[1].type = VM_INSTRUCTION_TYPE_WAIT;
  instructions[1].wait.duration = waitTime;
  instructions[2].type = VM_INSTRUCTION_TYPE_REPLACE_CONSOLE_MESSAGE;
  memset(instructions[2].consoleMessage.message, 0, sizeof(instructions[2].consoleMessage.message));
  strcpy(instructions[2].consoleMessage.message, message2);
  vmExecuter_init(instructions, 3);
}