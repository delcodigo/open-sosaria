#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "engine/engine.h"
#include "playerCommons.h"
#include "engine/input.h"
#include "data/player.h"
#include "data/bevery.h"
#include "entities/ui/uiztats.h"
#include "entities/ui/uiConsole.h"
#include "scenes/sceneDiskLoader.h"

PLAYER_STATE playerState = PLAYER_STATE_IDLE;
static READY_STEP readyStep = READY_STEP_START;
static Textfield inputTextfieldBuffer;
static bool areKeysReleased = true;
static char selectedWeapon[3] = {0};

bool playerCommons_updateZtats() {
  if (input.z == 1) {
    input.z = 2;
    uiZtats_init();
    ztatsActive = true;
    waitingTime = 0.0f;
    memset(&input, 0, sizeof(input));
    return true;
  }

  return false;
}

bool playerCommons_updateWait() {
  if (input.space) {
    waitingTime = 0.0f;
    char waitCommand[30] = {0};
    snprintf(waitCommand, sizeof(waitCommand), "%.14s%.15s", ultimaStrings[98], ultimaStrings[99]);
    uiConsole_replaceLastMessage(waitCommand);
    keyRepeatDelay = 0.3f;
    return true;
  }

  return false;
}

static void playerCommons_tryAndEquipSpell(int spellIndex, const char *charUsed) {
  if (spellIndex < 0) {
    uiConsole_replaceLastMessageFormat("%.15s%s%.10s", ultimaStrings[214], charUsed, ultimaStrings[224]);
    return;
  }

  if (spellIndex > 0 && player.spells[spellIndex - 1] < 1) {
    uiConsole_addMessageFormat("%.15s%s", ultimaStrings[225], spellNames[spellIndex]);
    return;
  }

  player.spell = spellIndex;
  uiConsole_addMessageFormat("%.15s%s", ultimaStrings[226], spellNames[spellIndex]);
}

static bool playerCommons_readyStart() {
  if (input.r == 1) {
    input.r = 2;
    waitingTime = 0.0f;
    memset(&input, 0, sizeof(input));
    char readyCommand[31] = {0};
    snprintf(readyCommand, sizeof(readyCommand), "%.14s%.15s", ultimaStrings[98], ultimaStrings[198]);
    uiConsole_replaceLastMessage(readyCommand);
    uiConsole_addMessage(ultimaStrings[199]);

    memset(&inputTextfieldBuffer, 0, sizeof(inputTextfieldBuffer));
    inputTextfieldBuffer.maxLength = 2;

    areKeysReleased = false;
    playerState = PLAYER_STATE_READY_TYPE;
    readyStep = READY_STEP_SELECT_TYPE;
  }

  return false;
}

static void playerCommons_activateInput() {
  inputTextfieldBuffer.active = true;
  inputTextfield = &inputTextfieldBuffer;
  inputTextfield->cursorPosition = 0;
  inputTextfield->text[0] = '\0';
  inputTextfield->isDirty = false;
  areKeysReleased = false;
}

static bool playerCommons_readySelectType() {
  if (input.w == 1) {
    input.w = 2;
    memset(selectedWeapon, 0, sizeof(selectedWeapon));
    uiConsole_addMessage(ultimaStrings[205]);
    playerCommons_activateInput();
    readyStep = READY_STEP_WEAPON_INPUT;
  } else if (input.a == 1) {
    input.a = 2;
    uiConsole_addMessage(ultimaStrings[210]);
    playerCommons_activateInput();
    readyStep = READY_STEP_ARMOR_EQUIP;
  } else if (input.s == 1) {
    input.s = 2;
    uiConsole_addMessage(ultimaStrings[214]);
    playerCommons_activateInput();
    readyStep = READY_STEP_SPELL_EQUIP;
  }

  return false;
}

static bool playerCommons_readyWeaponInput() {
  if (inputTextfield->isDirty && inputTextfield->text[0] != '\0') {
    inputTextfield->isDirty = false;

    if (selectedWeapon[0] == '\0') {
      selectedWeapon[0] = (char)toupper(inputTextfield->text[0]);
    } else if (selectedWeapon[1] == '\0') {
      selectedWeapon[1] = (char)toupper(inputTextfield->text[0]);
      readyStep = READY_STEP_WEAPON_EQUIP;
    }

    char weaponCommand[31] = {0};
    snprintf(weaponCommand, sizeof(weaponCommand), "%.14s%.2s", ultimaStrings[205], selectedWeapon);
    uiConsole_replaceLastMessage(weaponCommand);

    inputTextfield->cursorPosition = 0;
    inputTextfield->text[0] = '\0';
    areKeysReleased = false;
  }

  return false;
}

static bool playerCommons_readyWeaponEquip() {
  char weaponCommand[31] = {0};
  readyStep = READY_STEP_RETURN_TO_IDLE;
  areKeysReleased = false;

  for (int i=0;i<OS_WEAPONS_COUNT;i++) {
    char weaponAbbreviation[3] = {weaponNames[i][0], weaponNames[i][1], '\0'};
    if (strcmp(weaponAbbreviation, selectedWeapon) == 0) {
      memset(selectedWeapon, 0, sizeof(selectedWeapon));

      if (i > 0 && player.weapons[i - 1] < 1) {
        snprintf(weaponCommand, sizeof(weaponCommand), "%.15s%.15s", ultimaStrings[207], weaponNames[i]);
        uiConsole_addMessage(weaponCommand);
        return true;
      }

      player.weapon = i - 1;
      snprintf(weaponCommand, sizeof(weaponCommand), "%.15s%.15s", ultimaStrings[208], weaponNames[i]);
      uiConsole_addMessage(weaponCommand);
      return true;
    }
  }

  snprintf(weaponCommand, sizeof(weaponCommand), "%.2s%.15s", selectedWeapon, ultimaStrings[206]);
  uiConsole_addMessage(weaponCommand);
  memset(selectedWeapon, 0, sizeof(selectedWeapon));
  return true;
}

static bool playerCommons_readyArmorEquip() {
  if (inputTextfield->isDirty && inputTextfield->text[0] != '\0') {
    inputTextfield->isDirty = false;
    char armorChar[2] = {(char)toupper(inputTextfield->text[0]), '\0'};
    uiConsole_replaceLastMessageFormat("%.14s%.1s", ultimaStrings[210], armorChar);

    readyStep = READY_STEP_RETURN_TO_IDLE;
    areKeysReleased = false;

    for (int i=0;i<OS_ARMORS_COUNT;i++) {
      if (armorNames[i][0] == armorChar[0]) {
        if (player.armors[i - 1] < 1 && i > 0) {
          uiConsole_addMessageFormat("%.15s%.15s", ultimaStrings[212], armorNames[i]);
          return false;
        }

        uiConsole_addMessageFormat("%.15s%.15s", ultimaStrings[213], armorNames[i]);
        inputTextfield = NULL;
        player.armor = i - 1;
        return false;
      }
    }

    uiConsole_addMessageFormat("%.1s%.15s", armorChar, ultimaStrings[211]);
    return false;
  }

  return false;
}

static bool playerCommons_readySpellEquip() {
  if (inputTextfield->isDirty && inputTextfield->text[0] != '\0') {
    char selectedSpellChar[2] = {(char)toupper(inputTextfield->text[0]), '\0'};
    inputTextfield->isDirty = false;

    if (selectedSpellChar[0] == 'L') {
      uiConsole_replaceLastMessageFormat("%.15sL %.10s", ultimaStrings[214], ultimaStrings[216]);
      readyStep = READY_STEP_SPELL_LADDER;
      areKeysReleased = false;
      return false;
    } else if (selectedSpellChar[0] == 'P') {
      uiConsole_replaceLastMessageFormat("%.12sP[RAY/ROJ] (R/J)", ultimaStrings[214]);
      readyStep = READY_STEP_SPELL_PRAYER_OR_PROJECTILE;
      areKeysReleased = false;
      return false;
    }

    int spellIndex = -1;
    for (int i=0;i<OS_SPELLS_COUNT;i++) {
      if (spellNames[i][0] == selectedSpellChar[0]) {
        spellIndex = i;
        break;
      }
    }

    playerCommons_tryAndEquipSpell(spellIndex, selectedSpellChar);
    readyStep = READY_STEP_RETURN_TO_IDLE;
    areKeysReleased = false;
    return true;
  }

  return false;
}

static bool playerCommons_readySpellLadder() {
  if (inputTextfield->isDirty && inputTextfield->text[0] != '\0') {
    char selectedSpellChar[3] = {'L', (char)toupper(inputTextfield->text[0]), '\0'};
    inputTextfield->isDirty = false;
    uiConsole_replaceLastMessageFormat("%.15s%s", ultimaStrings[214], selectedSpellChar);

    readyStep = READY_STEP_RETURN_TO_IDLE;
    areKeysReleased = false;

    int spellIndex = -1;
    if (selectedSpellChar[1] == 'U') {
      spellIndex = 6;
      playerCommons_tryAndEquipSpell(spellIndex, "LU");
    } else if (selectedSpellChar[1] == 'D') {
      spellIndex = 5;
      playerCommons_tryAndEquipSpell(spellIndex, "LD");
    } else {
      uiConsole_addMessage(ultimaStrings[220]);
    }

    return true;
  }

  return false;
}

static void playerCommons_readyReturnToIdle() {
  inputTextfield = NULL;
  inputTextfieldBuffer.active = false;
  playerState = PLAYER_STATE_IDLE;
  readyStep = READY_STEP_START;
}

static bool playerCommons_readPrayerOrProjectile() {
  if (inputTextfield->isDirty && inputTextfield->text[0] != '\0') {
    char selectedSpellChar[3] = {'P', (char)toupper(inputTextfield->text[0]), '\0'};
    inputTextfield->isDirty = false;
    uiConsole_replaceLastMessageFormat("%.15s%s", ultimaStrings[214], selectedSpellChar);

    readyStep = READY_STEP_RETURN_TO_IDLE;
    areKeysReleased = false;

    int spellIndex = -1;
    if (selectedSpellChar[1] == 'R') {
      spellIndex = 0;
      playerCommons_tryAndEquipSpell(spellIndex, "PR");
    } else if (selectedSpellChar[1] == 'J') {
      spellIndex = 3;
      playerCommons_tryAndEquipSpell(spellIndex, "PJ");
    } else {
      uiConsole_addMessage(ultimaStrings[220]);
    }

    return true;
  }

  return false;
}

bool playerCommons_updateReady() {
  if (!areKeysReleased) {
    if (input_areKeysReleased()) {
      areKeysReleased = true;
      if (inputTextfield != NULL){
        inputTextfield->cursorPosition = 0;
        inputTextfield->text[0] = '\0';
      }
    } else {
      return false;
    }
  }

  switch (readyStep) {
    case READY_STEP_START:
      return playerCommons_readyStart();
    case READY_STEP_SELECT_TYPE:
      return playerCommons_readySelectType();
    case READY_STEP_WEAPON_INPUT:
      return playerCommons_readyWeaponInput();
    case READY_STEP_WEAPON_EQUIP:
      return playerCommons_readyWeaponEquip();
    case READY_STEP_ARMOR_EQUIP:
      return playerCommons_readyArmorEquip();
    case READY_STEP_SPELL_EQUIP:
      return playerCommons_readySpellEquip();
    case READY_STEP_SPELL_LADDER:
      return playerCommons_readySpellLadder();
    case READY_STEP_SPELL_PRAYER_OR_PROJECTILE:
      return playerCommons_readPrayerOrProjectile();
      break;
    case READY_STEP_RETURN_TO_IDLE:
      playerCommons_readyReturnToIdle();
      break;
  }

  return false;
}