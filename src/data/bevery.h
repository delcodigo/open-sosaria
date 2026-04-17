#ifndef OS_BEVERY_H
#define OS_BEVERY_H

#define OS_DUNGEON_TABLE_HEIGHT 11

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

extern int dungeonTable[OS_DUNGEON_TABLE_HEIGHT][4];
extern int dungeonDoorsTable[OS_DUNGEON_TABLE_HEIGHT][6];
extern int dungeonDoorsFrontTable[OS_DUNGEON_TABLE_HEIGHT][4];
extern int dungeonTrapsTable[OS_DUNGEON_TABLE_HEIGHT][6];
extern int dungeonLaddersTable[OS_DUNGEON_TABLE_HEIGHT][4];

void bevery_free();

#endif