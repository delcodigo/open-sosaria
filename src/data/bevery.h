#ifndef OS_BEVERY_H
#define OS_BEVERY_H

#define OS_DUNGEON_TABLE_HEIGHT 11
#define OS_DUNGEON_TABLE_WIDTH 4
#define OS_DUNGEON_DOORS_TABLE_WIDTH 6
#define OS_DUNGEON_DOORS_FRONT_TABLE_WIDTH 4

extern char **statsNames;
extern int statsNamesSize;

extern char **armorNames;
extern int armorNamesSize;

extern char **vehicleNames;
extern int vehicleNamesSize;

extern char **weaponNames;
extern int weaponNamesSize;

extern char **spellNames;
extern int spellNamesSize;

extern char **racesNames;
extern int racesNamesSize;

extern char **typesNames;
extern int typesNamesSize;

extern char **placesNames;
extern int placesNamesSize;

extern int dungeonTable[OS_DUNGEON_TABLE_HEIGHT][OS_DUNGEON_TABLE_WIDTH];
extern int dungeonDoorsTable[OS_DUNGEON_TABLE_HEIGHT][OS_DUNGEON_DOORS_TABLE_WIDTH];
extern int dungeonDoorsFrontTable[OS_DUNGEON_TABLE_HEIGHT][OS_DUNGEON_DOORS_FRONT_TABLE_WIDTH];

void bevery_free();

#endif