#include <stdlib.h>
#include "vmExecuter.h"
#include "engine/scene.h"

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