#ifndef OS_PLAYER_COMMONS_H
#define OS_PLAYER_COMMONS_H

#include <stdbool.h>
#include "engine/input.h"

typedef enum {
  PLAYER_STATE_IDLE,
  PLAYER_STATE_READY_TYPE
} PLAYER_STATE;

typedef enum {
  READY_STEP_START = 0,
  READY_STEP_SELECT_TYPE = 1,
  READY_STEP_WEAPON_INPUT = 2,
  READY_STEP_WEAPON_EQUIP = 3,
  READY_STEP_ARMOR_EQUIP = 4,
  READY_STEP_SPELL_EQUIP = 5,
  READY_STEP_SPELL_LADDER = 6,
  READY_STEP_SPELL_PRAYER_OR_PROJECTILE = 7,
  READY_STEP_RETURN_TO_IDLE = 8
} READY_STEP;

extern PLAYER_STATE playerState;

bool playerCommons_updateZtats();
bool playerCommons_updateWait();
bool playerCommons_updateReady();

#endif