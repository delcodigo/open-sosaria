#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "saveAndLoad.h"
#include "entities/vehicleOverworld.h"

static char saveHeader[] = "--OPEN SOSARIA--";
static char saveVersion[] = "v0.1.0";
SaveData savedGame = { 0 };

bool loadGame() {
  FILE* file = fopen("PLAYER.BIN", "rb");
  if (file == NULL) {
    perror("Failed to open file for loading");
    return false;
  }

  fread(&savedGame, sizeof(SaveData), 1, file);
  fclose(file);

  if (strcmp(savedGame.header, saveHeader) != 0 || strcmp(savedGame.version, saveVersion) != 0) {
    fprintf(stderr, "Invalid save file format or version\n");
    return false;
  }

  memcpy(&player, &savedGame.player, sizeof(Player));

  memcpy(vehiclesMap, savedGame.vehiclesMap, sizeof(vehiclesMap));

  return true;
}

void saveGame() {
  FILE* file = fopen("PLAYER.BIN", "wb");
  if (file == NULL) {
    perror("Failed to open file for saving");
    return;
  }

  memset(&savedGame, 0, sizeof(SaveData));
  strncpy(savedGame.header, saveHeader, sizeof(savedGame.header) - 1);
  strncpy(savedGame.version, saveVersion, sizeof(savedGame.version) - 1);

  memcpy(&savedGame.player, &player, sizeof(Player));

  memcpy(savedGame.vehiclesMap, vehiclesMap, sizeof(vehiclesMap));

  fwrite(&savedGame, sizeof(SaveData), 1, file);
  fclose(file);
}