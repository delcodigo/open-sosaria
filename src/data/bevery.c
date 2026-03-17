#include <stdlib.h>
#include "bevery.h"
#include "memory.h"

char **statsNames = NULL;
int statsNamesSize = 0;

char **armorNames = NULL;
int armorNamesSize = 0;

char **vehicleNames = NULL;
int vehicleNamesSize = 0;

char **weaponNames = NULL;
int weaponNamesSize = 0;

char **spellNames = NULL;
int spellNamesSize = 0;

char **racesNames = NULL;
int racesNamesSize = 0;

char **typesNames = NULL;
int typesNamesSize = 0;

void bevery_free() {
  for (int i=0;i<statsNamesSize;i++) {
    free(statsNames[i]);
  }
  free(statsNames);

  for (int i=0;i<armorNamesSize;i++) {
    free(armorNames[i]);
  }
  free(armorNames);

  for (int i=0;i<vehicleNamesSize;i++) {
    free(vehicleNames[i]);
  }
  free(vehicleNames);

  for (int i=0;i<weaponNamesSize;i++) {
    free(weaponNames[i]);
  }
  free(weaponNames);

  for (int i=0;i<spellNamesSize;i++) {
    free(spellNames[i]);
  }
  free(spellNames);

  for (int i=0;i<racesNamesSize;i++) {
    free(racesNames[i]);
  }
  free(racesNames);

  for (int i=0;i<typesNamesSize;i++) {
    free(typesNames[i]);
  }
  free(typesNames);

  statsNames = NULL;
  statsNamesSize = 0;
  armorNames = NULL;
  armorNamesSize = 0;
  vehicleNames = NULL;
  vehicleNamesSize = 0;
  weaponNames = NULL;
  weaponNamesSize = 0;
  spellNames = NULL;
  spellNamesSize = 0;
  racesNames = NULL;
  racesNamesSize = 0;
  typesNames = NULL;
  typesNamesSize = 0;
}