#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "sceneDiskLoader.h"
#include "engine/text.h"
#include "utils.h"
#include "memory.h"
#include "config.h"

#define DISK_SIZE 35 * 16 * 256
#define DOS_TRACKS 35
#define DOS_SECTORS 16
#define DOS_SECTOR_SIZE 256

static Text diskMsg;
static char diskMsgText[41] = {0};

typedef struct {
  unsigned char *data;
  unsigned int size;
} Buffer;

typedef struct {
  char name[31];
  unsigned char type;
  unsigned int size;
} DosFile;

typedef struct {
  const char *name;
  unsigned int expectedSize;
  int disk;
} FileRequirement;

unsigned char *sceneDiskLoader_getDos33Sector(unsigned char *disk, int track, int sector) {
  if (track < 0 || track >= DOS_TRACKS || sector < 0 || sector >= DOS_SECTORS) {
    return NULL;
  }

  int offset = (track * DOS_SECTORS + sector) * DOS_SECTOR_SIZE;
  if (offset < 0 || offset + DOS_SECTOR_SIZE > DISK_SIZE) {
    return NULL;
  }

  return disk + offset;
}

char *sceneDiskLoader_decodeDosFilename(unsigned char *nameBytes) {
  static char name[31];
  memset(name, 0, sizeof(name));

  for (int i=0;i<30;i++) {
    unsigned char byte = nameBytes[i];
    char c = (byte & 0x7f) ? (char)(byte & 0x7f) : ' ';
    name[i] = (c >= 32 && c <= 126) ? c : ' ';
  }

  // Trim trailing spaces
  int len = strlen(name);
  while (len > 0 && name[len-1] == ' ') {
    name[--len] = '\0';
  }

  return name;
}

Buffer *sceneDiskLoader_readDos33FileFromTsList(unsigned char *disk, int tsTrack, int tsSector, unsigned int maxSectors) {
  unsigned char *data = (unsigned char *) malloc(maxSectors * DOS_SECTOR_SIZE);
  if (!data) { return NULL; }

  unsigned int total = 0;
  int visited[DOS_TRACKS][DOS_SECTORS];
  memset(visited, 0, sizeof(visited));

  int track = tsTrack;
  int sector = tsSector;

  while (track != 0 && total < maxSectors) {
    if (visited[track][sector]) { break; }
    visited[track][sector] = 1;

    unsigned char *ts = sceneDiskLoader_getDos33Sector(disk, track, sector);
    if (!ts) { break; }

    int nextTrack = ts[1];
    int nextSector = ts[2];

    for (int o=0x0c;o<0x100 && total<maxSectors;o+=2) {
      unsigned char dt = ts[o];
      unsigned char ds = ts[o+1];
      if (dt == 0) { break; }

      unsigned char *sectorData = sceneDiskLoader_getDos33Sector(disk, dt, ds);
      if (!sectorData) { continue; }

      memcpy(data + total * DOS_SECTOR_SIZE, sectorData, DOS_SECTOR_SIZE);
      total++;
    }

    track = nextTrack;
    sector = nextSector;
  }

  Buffer *buffer = (Buffer *) malloc(sizeof(Buffer));
  if (!buffer) {
    free(data);
    return NULL;
  }

  buffer->data = data;
  buffer->size = total * DOS_SECTOR_SIZE;

  return buffer;
}

static DosFile *sceneDiskLoader_readDos33Catalog(unsigned char *disk, int *outCount) {
  if (!disk) { return NULL; }

  unsigned char *vtoc = sceneDiskLoader_getDos33Sector(disk, 17, 0);
  if (!vtoc) { return NULL; }

  DosFile *file = (DosFile *) malloc(sizeof(DosFile) * 200);
  if (!file) { return NULL; }

  int fileCount = 0;
  int catTrack = vtoc[1];
  int catSector = vtoc[2];

  int visited[DOS_TRACKS][DOS_SECTORS];
  memset(visited, 0, sizeof(visited));

  while (catTrack != 0) {
    if (visited[catTrack][catSector]) { break; }
    visited[catTrack][catSector] = 1;

    unsigned char *catalogSector = sceneDiskLoader_getDos33Sector(disk, catTrack, catSector);
    if (!catalogSector) { break; }

    for (int i=0;i<7;i++) {
      int entryOffset = 0x0b + i *35;
      unsigned char tsTrack = catalogSector[entryOffset];
      unsigned char tsSector = catalogSector[entryOffset + 1];
      unsigned char fileType = catalogSector[entryOffset + 2] & 0x7f;
      unsigned char *nameBytes = catalogSector + entryOffset + 3;
      unsigned short lenSectors = catalogSector[entryOffset + 33] | (catalogSector[entryOffset + 34] << 8);

      if (!nameBytes[0] || tsTrack == 0 || tsTrack == 0xff) { continue; }

      char *decodedName = sceneDiskLoader_decodeDosFilename(nameBytes);
      if (!decodedName || strlen(decodedName) == 0) { continue; }

      // Read the file
      Buffer *fileData = sceneDiskLoader_readDos33FileFromTsList(disk, tsTrack, tsSector, lenSectors ? lenSectors : 65535);
      if (!fileData) { continue; }

      strcpy(file[fileCount].name, decodedName);
      file[fileCount].size = fileData->size;
      file[fileCount].type = fileType;
      fileCount++;

      free(fileData->data);
      free(fileData);
    }

    catTrack = catalogSector[1];
    catSector = catalogSector[2];
  }

  *outCount = fileCount;
  return file;
}

static DosFile *sceneDiskLoader_findFileByName(DosFile *files, int count, const char *name) {
  for (int i=0;i<count;i++) {
    if (strcmp(files[i].name, name) == 0) {
      return &files[i];
    }
  }
  return NULL;
}

static int sceneDiskLoader_verifyUltimaDisks() {
  if (!file_exists("disk1.dsk")) {
    strcpy(diskMsgText, "'disk1.dsk' not found!");
    text_create(&diskMsg, diskMsgText);
    return 0;
  }

  if (!file_exists("disk2.dsk")) {
    strcpy(diskMsgText, "'disk2.dsk' not found!");
    text_create(&diskMsg, diskMsgText);
    return 0;
  }

  unsigned char *disk1 = (unsigned char *)malloc(DISK_SIZE);
  unsigned char *disk2 = (unsigned char *)malloc(DISK_SIZE);

  if (!disk1 || !disk2) {
    strcpy(diskMsgText, "Failed to allocate memory for disks!");
    text_create(&diskMsg, diskMsgText);
    free(disk1);
    free(disk2);
    return 0;
  }

  FILE *file1 = fopen("disk1.dsk", "rb");
  FILE *file2 = fopen("disk2.dsk", "rb");

  fread(disk1, 1, DISK_SIZE, file1);
  fread(disk2, 1, DISK_SIZE, file2);
  fclose(file1);
  fclose(file2);

  // Read catalogs
  int disk1Count = 0, disk2Count = 0;
  DosFile *disk1Files = sceneDiskLoader_readDos33Catalog(disk1, &disk1Count);
  DosFile *disk2Files = sceneDiskLoader_readDos33Catalog(disk2, &disk2Count);

  if (!disk1Files || !disk2Files) {
    strcpy(diskMsgText, "Failed to read disk catalogs!");
    text_create(&diskMsg, diskMsgText);
    free(disk1);
    free(disk2);
    free(disk1Files);
    free(disk2Files);
    return 0;
  }

  // Required files
  FileRequirement requiredDisk1[] = {
    {"PIC.ULTIMATUM", 8192, 1},
    {"OUT.SHAPES", 768, 1},
    {"TWN.CAS.SHAPES", 512, 1},
    {"SPA.SHAPES", 1024, 1},
    {"ULTSHAPES", 768, 1},
    {"DRAW 64.OBJ", 768, 1},
    {"OUT MOVE", 11008, 1},
    {"TWN MOVE", 14336, 1},
    {"CAS MOVE", 12032, 1},
    {"DNG MOVE 1", 11520, 1},
    {"SET1", 2560, 1},
    {"SET2", 2560, 1},
    {"SET3", 2560, 1},
    {"SET4", 2560, 1},
    {"SET5", 2304, 1},
    {NULL, 0, 0}
  };

  FileRequirement requiredDisk2[] = {
    {"TWN.PIC", 8192, 2},
    {"CAS.PIC", 8192, 2},
    {"BTERRA0", 4352, 2},
    {"BTERRA1", 4352, 2},
    {"BTERRA2", 4352, 2},
    {"BTERRA3", 4352, 2},
    {"BTWN", 1024, 2},
    {"BCAS", 1024, 2},
    {NULL, 0, 0}
  };

  // Check disk 1 files
  for (int i=0;requiredDisk1[i].name != NULL;i++) {
    DosFile *file = sceneDiskLoader_findFileByName(disk1Files, disk1Count, requiredDisk1[i].name);

    if (!file) {
      snprintf(diskMsgText, sizeof(diskMsgText), "File '%s' missing on disk 1!", requiredDisk1[i].name);
      text_create(&diskMsg, diskMsgText);
      free(disk1);
      free(disk2);
      free(disk1Files);
      free(disk2Files);
      return 0;
    } else if (file->size != requiredDisk1[i].expectedSize) {
      snprintf(diskMsgText, sizeof(diskMsgText), "'%s' on disk 1 is incorrect!", requiredDisk1[i].name);
      text_create(&diskMsg, diskMsgText);
      free(disk1);
      free(disk2);
      free(disk1Files);
      free(disk2Files);
      return 0;
    }
  }

  // Check disk 2 files
  for (int i=0;requiredDisk2[i].name != NULL;i++) {
    DosFile *file = sceneDiskLoader_findFileByName(disk2Files, disk2Count, requiredDisk2[i].name);

    if (!file) {
      snprintf(diskMsgText, sizeof(diskMsgText), "File '%s' missing on disk 2!", requiredDisk2[i].name);
      text_create(&diskMsg, diskMsgText);
      free(disk1);
      free(disk2);
      free(disk1Files);
      free(disk2Files);
      return 0;
    } else if (file->size != requiredDisk2[i].expectedSize) {
      snprintf(diskMsgText, sizeof(diskMsgText), "'%s' on disk 2 is incorrect!", requiredDisk2[i].name);
      text_create(&diskMsg, diskMsgText);
      free(disk1);
      free(disk2);
      free(disk1Files);
      free(disk2Files);
      return 0;
    }
  }

  strcpy(diskMsgText, "ULTIMA Disks Verified!");
  text_create(&diskMsg, diskMsgText);
  free(disk1);
  free(disk2);
  free(disk1Files);
  free(disk2Files);

  return 1;
}

void sceneDiskLoader_init() {
  sceneDiskLoader_verifyUltimaDisks();
}

void sceneDiskLoader_update() {
  int x = (OS_SCREEN_WIDTH - (strlen(diskMsgText) * OS_FONT_GLYPH_WIDTH)) / 2;
  text_render(&diskMsg, x, OS_SCREEN_HEIGHT / 2);
}

void sceneDiskLoader_free() {
  text_free(&diskMsg);
}

Scene sceneDiskLoader = {
  .scene_init = sceneDiskLoader_init,
  .scene_update = sceneDiskLoader_update,
  .scene_free = sceneDiskLoader_free
};
