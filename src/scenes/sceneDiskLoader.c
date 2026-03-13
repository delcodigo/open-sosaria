#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "engine/text.h"
#include "engine/texture.h"
#include "sceneDiskLoader.h"
#include "maths/matrix4.h"
#include "utils.h"
#include "memory.h"
#include "config.h"
#include "sceneSplash.h"

#define DISK_SIZE 35 * 16 * 256
#define DOS_TRACKS 35
#define DOS_SECTORS 16
#define DOS_SECTOR_SIZE 256

static Text diskMsg;
static char diskMsgText[41] = {0};
char ultimaStrings[1000][41];
static int ultimaStringCount = 0;
UltimaAssets ultimaAssets = {0};
float loaderTime = 0.0f;

typedef struct {
  uint8_t *data;
  uint32_t size;
} Buffer;

typedef struct {
  char name[31];
  uint8_t type;
  uint32_t size;
} DosFile;

typedef struct {
  const char *name;
  uint32_t expectedSize;
  int disk;
} FileRequirement;

uint8_t *sceneDiskLoader_getDos33Sector(uint8_t *disk, int track, int sector) {
  if (track < 0 || track >= DOS_TRACKS || sector < 0 || sector >= DOS_SECTORS) {
    return NULL;
  }

  int offset = (track * DOS_SECTORS + sector) * DOS_SECTOR_SIZE;
  if (offset < 0 || offset + DOS_SECTOR_SIZE > DISK_SIZE) {
    return NULL;
  }

  return disk + offset;
}

char *sceneDiskLoader_decodeDosFilename(uint8_t *nameBytes) {
  static char name[31];
  memset(name, 0, sizeof(name));

  for (int i=0;i<30;i++) {
    uint8_t byte = nameBytes[i];
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

Buffer *sceneDiskLoader_readDos33FileFromTsList(uint8_t *disk, int tsTrack, int tsSector, uint32_t maxSectors) {
  uint8_t *data = (uint8_t *) malloc(maxSectors * DOS_SECTOR_SIZE);
  if (!data) { return NULL; }

  uint32_t total = 0;
  int visited[DOS_TRACKS][DOS_SECTORS];
  memset(visited, 0, sizeof(visited));

  int track = tsTrack;
  int sector = tsSector;

  while (track != 0 && total < maxSectors) {
    if (visited[track][sector]) { break; }
    visited[track][sector] = 1;

    uint8_t *ts = sceneDiskLoader_getDos33Sector(disk, track, sector);
    if (!ts) { break; }

    int nextTrack = ts[1];
    int nextSector = ts[2];

    for (int o=0x0c;o<0x100 && total<maxSectors;o+=2) {
      uint8_t dt = ts[o];
      uint8_t ds = ts[o+1];
      if (dt == 0) { break; }

      uint8_t *sectorData = sceneDiskLoader_getDos33Sector(disk, dt, ds);
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

static Buffer *sceneDiskLoader_readDos33FileByName(uint8_t *disk, const char *filename) {
  if (!disk || !filename) { return NULL; }

  uint8_t *vtoc = sceneDiskLoader_getDos33Sector(disk, 17, 0);
  if (!vtoc) { return NULL; }

  int catTrack = vtoc[1];
  int catSector = vtoc[2];
  int visited[DOS_TRACKS][DOS_SECTORS];
  memset(visited, 0, sizeof(visited));

  while (catTrack != 0) {
    if (catTrack < 0 || catTrack >= DOS_TRACKS || catSector < 0 || catSector >= DOS_SECTORS) {
      break;
    }

    if (visited[catTrack][catSector]) { break; }
    visited[catTrack][catSector] = 1;

    uint8_t *catalogSector = sceneDiskLoader_getDos33Sector(disk, catTrack, catSector);
    if (!catalogSector) { break; }

    for (int i=0;i<7;i++) {
      int entryOffset = 0x0b + i *35;
      uint8_t tsTrack = catalogSector[entryOffset];
      uint8_t tsSector = catalogSector[entryOffset + 1];
      uint8_t *nameBytes = catalogSector + entryOffset + 3;
      uint16_t lenSectors = catalogSector[entryOffset + 33] | (catalogSector[entryOffset + 34] << 8);

      if (!nameBytes[0] || tsTrack == 0 || tsTrack == 0xff) { continue; }

      char *decodedName = sceneDiskLoader_decodeDosFilename(nameBytes);
      if (!decodedName || strlen(decodedName) == 0) { continue; }

      if (strcmp(decodedName, filename) == 0) {
        return sceneDiskLoader_readDos33FileFromTsList(disk, tsTrack, tsSector, lenSectors ? lenSectors : 65535);
      }
    }

    catTrack = catalogSector[1];
    catSector = catalogSector[2];
  }

  return NULL;
}

static DosFile *sceneDiskLoader_readDos33Catalog(uint8_t *disk, int *outCount) {
  if (!disk) { return NULL; }

  uint8_t *vtoc = sceneDiskLoader_getDos33Sector(disk, 17, 0);
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

    uint8_t *catalogSector = sceneDiskLoader_getDos33Sector(disk, catTrack, catSector);
    if (!catalogSector) { break; }

    for (int i=0;i<7;i++) {
      int entryOffset = 0x0b + i *35;
      uint8_t tsTrack = catalogSector[entryOffset];
      uint8_t tsSector = catalogSector[entryOffset + 1];
      uint8_t fileType = catalogSector[entryOffset + 2] & 0x7f;
      uint8_t *nameBytes = catalogSector + entryOffset + 3;
      uint16_t lenSectors = catalogSector[entryOffset + 33] | (catalogSector[entryOffset + 34] << 8);

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

static const uint8_t *sceneDiskLoader_maybeStripBloadHeader(const uint8_t *data, uint32_t size, uint32_t *outSize) {
  if (!data || !outSize) { return NULL; }

  if (size < 8) {
    *outSize = size;
    return data;
  }

  uint16_t bodyLength = (uint16_t)(data[2] | (data[3] << 8));
  if (bodyLength > 0 && (uint32_t)bodyLength + 4 <= size) {
    *outSize = bodyLength;
    return data + 4;
  }

  *outSize = size;
  return data;
}

static UltimaImage sceneDiskLoader_createImage(uint32_t width, uint32_t height, uint8_t bg_r, uint8_t bg_g, uint8_t bg_b) {
  UltimaImage img;
  img.width = width;
  img.height = height;
  img.data = (uint8_t *) malloc(width * height * 4);

  if (img.data) {
    for (uint32_t i=0;i<width * height;i++) {
      img.data[i*4 + 0] = bg_r;
      img.data[i*4 + 1] = bg_g;
      img.data[i*4 + 2] = bg_b;
      img.data[i*4 + 3] = 255;
    }
  }

  return img;
}

static void sceneDiskLoader_freeImage(UltimaImage *img) {
  if (img && img->data) {
    free(img->data);
    img->data = NULL;
    img->width = 0;
    img->height = 0;
  }
}

static void sceneDiskLoader_setPixel(UltimaImage *img, int x, int y, uint8_t r, uint8_t g, uint8_t b) {
  if (!img || !img->data) { return; }
  if (x < 0 || (uint32_t)x >= img->width || y < 0 || (uint32_t)y >= img->height) { return; }

  uint32_t index = (y * img->width + x) * 4;
  img->data[index + 0] = r;
  img->data[index + 1] = g;
  img->data[index + 2] = b;
  img->data[index + 3] = 255;
}

static bool sceneDiskLoader_decodeHGRImage(const uint8_t *dataRaw, uint32_t sizeRaw, UltimaImage *outImage) {
  if (!dataRaw || !outImage) { return false; }

  uint32_t size = 0;
  const uint8_t *data = sceneDiskLoader_maybeStripBloadHeader(dataRaw, sizeRaw, &size);
  if (!data || size < 8184) { return false; }

  const int originalWidth = 280;
  const int originalHeight = 192;

  *outImage = sceneDiskLoader_createImage(originalWidth, originalHeight, 0, 0, 0);
  if (!outImage->data) { return false; }

  // Decode HGR with approximate colors
  for (int y=0;y<originalHeight;y++) {
    int rowBase = ((y & 0x07) * 0x400) + (((y >> 3) & 0x07) * 0x80) + ((y >> 6) * 0x28);

    // first pass: unpack whole scanline
    uint8_t bits[280] = {0};
    uint8_t phase[280] = {0};

    for (int xb = 0; xb < 40; xb++) {
        int addr = rowBase + xb;
        if (addr >= (int)size) continue;

        uint8_t byte = data[addr];
        uint8_t ph = (byte >> 7) & 1;

        for (int bit = 0; bit < 7; bit++) {
          int x = xb * 7 + bit;
          bits[x] = (byte >> bit) & 1;
          phase[x] = ph;
        }
    }

    // second pass: decode with neighbors
    for (int x = 0; x < 280; x++) {
        int left = (x > 0) ? bits[x - 1] : 0;
        int current = bits[x];
        int right = (x < 279) ? bits[x + 1] : 0;

        uint8_t r = 0, g = 0, b = 0;

        if (!current) {
          // black
          r = g = b = 0;
        } else if (left || right) {
          // adjacent lit pixels trend toward white
          r = g = b = 255;
        } else {
          // isolated pixel -> artifact color
          int odd = x & 1;
          int ph = phase[x];

          // phase shifts the palette
          // ph=0: odd/even -> green/purple
          // ph=1: odd/even -> orange/blue
          if (ph == 0) {
            if (odd) {
              r = 48; g = 200; b = 64; // green
            } else {
              r = 200; g = 64; b = 255; // purple
            }
          } else {
            if (odd) {
              r = 255; g = 144; b = 48; // orange
            } else {
              r = 64; g = 136; b = 255; // blue
            }
          }
        }

        sceneDiskLoader_setPixel(outImage, x, y, r, g, b);
    }
  }

  outImage->textureId = texture_load(outImage->width, outImage->height, outImage->data);
  sceneDiskLoader_freeImage(outImage);

  return true;
}

static void sceneDiskLoader_addToUltimaStrings(const char *str, int length) {
  if (ultimaStringCount < 1000) {
    strncpy(ultimaStrings[ultimaStringCount], str, length);
    printf("Added to strings: [%d] '%s'\n", ultimaStringCount, ultimaStrings[ultimaStringCount]);
    ultimaStringCount++;
  }
}

static void sceneDiskLoader_emitQuotedStringsFromAppleBasicLine(const unsigned char *lineData, size_t lineLength) {
  size_t i = 0;
  bool shouldAppend = false;

  while (i<lineLength) {
    if (lineData[i] == '"') {
      size_t start = i + 1;
      size_t end = start;

      while (end < lineLength && lineData[end] != '"') {
        end++;
      }

      if (end >= lineLength) { return; }

      size_t length = end - start;
      if (length > 0 && length < 41) {
        if (shouldAppend) {
          size_t existingLength = strlen(ultimaStrings[ultimaStringCount]);
          if (existingLength + length < 41) {
            strncat(ultimaStrings[ultimaStringCount], (const char *)(lineData + start), length);
            ultimaStrings[ultimaStringCount][existingLength + length] = '\0';
            printf("Appended to string: [%d] '%s'\n", ultimaStringCount, ultimaStrings[ultimaStringCount]);
            shouldAppend = false;
            ultimaStringCount++;
            i = end + 1;
            continue;
          }
        } else {
          memcpy(ultimaStrings[ultimaStringCount], lineData + start, length);
          ultimaStrings[ultimaStringCount][length] = '\0';
        }


        if (lineData[end + 1] == 200) { // '+' concatenation operator
          shouldAppend = true;
        } else {
          ultimaStringCount++;
          shouldAppend = false;
          printf("Extracted string: [%d] '%s'\n", ultimaStringCount - 1, ultimaStrings[ultimaStringCount - 1]);
        }
      }

      i = end + 1;
      continue;
    }
    
    i++;
  }
}

static void sceneDiskLoader_hgrColorForPixel(bool on, bool hi, bool parityEven, bool adjacentOn, uint8_t *r, uint8_t *g, uint8_t *b) {
  if (!on) {
    *r = 0; *g = 0; *b = 0;
  } else if (adjacentOn) {
    *r = 245; *g = 245; *b = 245;
  } else if (hi) {
    if (parityEven) {
      *r = 70; *g = 150; *b = 255;
    } else {
      *r = 255; *g = 150; *b = 70;
    }
  } else {
    if (parityEven) {
      *r = 190; *g = 80; *b = 255;
    } else {
      *r = 80; *g = 220; *b = 80;
    }
  }
}

static void sceneDiskLoader_decodeUltShapesTiles(const uint8_t *dataRaw, uint32_t sizeRaw, UltimaImage *outImage) {
  if (!dataRaw || !outImage) { return; }

  uint32_t size = 0;
  const uint8_t *data = sceneDiskLoader_maybeStripBloadHeader(dataRaw, sizeRaw, &size);
  if (!data || size < 0x210) { return; }

  const int bankA = 0x100;
  const int bankB = 0x000;
  const int pageA = size - bankA < 256 ? size - bankA : 256;
  const int pageB = size - bankB < 256 ? size - bankB : 256;
  const int tileCount = (pageA < pageB ? pageA : pageB) / 16;

  if (tileCount <= 0) { return; }

  const int tileW = 14;
  const int tileH = 16;
  const int tilesPerRow = tileCount < 8 ? tileCount : 8;
  const int rows = (tileCount + tilesPerRow - 1) / tilesPerRow;
  const int originalWidth = tilesPerRow * tileW;
  const int originalHeight = rows * tileH;

  *outImage = sceneDiskLoader_createImage(originalWidth, originalHeight, 0, 0, 0);
  if (!outImage->data) { return; }

  for (int t=0;t<tileCount;t++) {
    int tx = t % tilesPerRow;
    int ty = t / tilesPerRow;
    int ox = tx * tileW;
    int oy = ty * tileH;

    for (int y=0;y<tileH;y++) {
      uint8_t b1 = (bankA + t * 16 + y < (int)size) ? data[bankA + t * 16 + y] : 0;
      uint8_t b2 = (bankB + t * 16 + y < (int)size) ? data[bankB + t * 16 + y] : 0;

      bool on[14] = {0};
      bool hi[14] = {0};

      for (int i=0;i<7;i++) {
        on[i] = ((b1 >> i) & 1) != 0;
        hi[i] = (b1 & 0x80) != 0;
        on[7 + i] = ((b2 >> i) & 1) != 0;
        hi[7 + i] = (b2 & 0x80) != 0;
      }

      for (int x=0;x<tileW;x++) {
        bool adj = (x>0 && on[x-1]) || (x<13 && on[x+1]);
        uint8_t r = 0, g = 0, b = 0;
        sceneDiskLoader_hgrColorForPixel(on[x], hi[x], (x & 1) == 0, adj, &r, &g, &b);

        sceneDiskLoader_setPixel(outImage, ox + x, oy + y, r, g, b);
      }
    }
  }

  outImage->textureId = texture_load(outImage->width, outImage->height, outImage->data);
  sceneDiskLoader_freeImage(outImage);
  outImage->width = originalWidth;
  outImage->height = originalHeight;
}

static void sceneDiskLoader_decodeBterraMap(const uint8_t *dataRaw, uint32_t sizeRaw, uint8_t map[OS_BTERRA_MAP_WIDTH][OS_BTERRA_MAP_HEIGHT]) {
  if (!dataRaw || !map) { return; }

  uint32_t size = 0;
  const uint8_t *data = sceneDiskLoader_maybeStripBloadHeader(dataRaw, sizeRaw, &size);
  if (!data || size < OS_BTERRA_MAP_WIDTH * OS_BTERRA_MAP_HEIGHT) { return; }

  for (int y=0;y<OS_BTERRA_MAP_HEIGHT;y++) {
    for (int x=0;x<OS_BTERRA_MAP_WIDTH;x++) {
      int idx = y * OS_BTERRA_MAP_WIDTH + x;
      map[y][x] = (uint8_t)((data[idx] >> 4) & 0x0F);
    }
  }
}

static int sceneDiskLoader_verifyUltimaDisks() {
  if (!file_exists("disk1.dsk")) {
    strcpy(diskMsgText, "'disk1.dsk' not found!");
    text_create(&diskMsg, diskMsgText, false);
    return 0;
  }

  if (!file_exists("disk2.dsk")) {
    strcpy(diskMsgText, "'disk2.dsk' not found!");
    text_create(&diskMsg, diskMsgText, false);
    return 0;
  }

  uint8_t *disk1 = (uint8_t *)malloc(DISK_SIZE);
  uint8_t *disk2 = (uint8_t *)malloc(DISK_SIZE);

  if (!disk1 || !disk2) {
    strcpy(diskMsgText, "Failed to allocate memory for disks!");
    text_create(&diskMsg, diskMsgText, false);
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
    text_create(&diskMsg, diskMsgText, false);
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
      text_create(&diskMsg, diskMsgText, false);
      free(disk1);
      free(disk2);
      free(disk1Files);
      free(disk2Files);
      return 0;
    } else if (file->size != requiredDisk1[i].expectedSize) {
      snprintf(diskMsgText, sizeof(diskMsgText), "'%s' on disk 1 is incorrect!", requiredDisk1[i].name);
      text_create(&diskMsg, diskMsgText, false);
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
      text_create(&diskMsg, diskMsgText, false);
      free(disk1);
      free(disk2);
      free(disk1Files);
      free(disk2Files);
      return 0;
    } else if (file->size != requiredDisk2[i].expectedSize) {
      snprintf(diskMsgText, sizeof(diskMsgText), "'%s' on disk 2 is incorrect!", requiredDisk2[i].name);
      text_create(&diskMsg, diskMsgText, false);
      free(disk1);
      free(disk2);
      free(disk1Files);
      free(disk2Files);
      return 0;
    }
  }

  strcpy(diskMsgText, "--OPEN SOSARIA--");
  text_create(&diskMsg, diskMsgText, false);
  free(disk1);
  free(disk2);
  free(disk1Files);
  free(disk2Files);

  return 1;
}

void sceneDiskLoader_extractUltimaAssets() {
  memset(&ultimaAssets, 0, sizeof(UltimaAssets));

  uint8_t *disk1 = (uint8_t *)malloc(DISK_SIZE);
  uint8_t *disk2 = (uint8_t *)malloc(DISK_SIZE);

  if (!disk1) { return; }
  if (!disk2) {
    free(disk1);
    return;
  }

  FILE *file1 = fopen("disk1.dsk", "rb");
  FILE *file2 = fopen("disk2.dsk", "rb");

  fread(disk1, 1, DISK_SIZE, file1);
  fread(disk2, 1, DISK_SIZE, file2);
  fclose(file1);
  fclose(file2);


  Buffer *titleBuffer = sceneDiskLoader_readDos33FileByName(disk1, "PIC.ULTIMATUM");
  if (titleBuffer && titleBuffer->data) {
    // Decode HGR image
    if (!sceneDiskLoader_decodeHGRImage(titleBuffer->data, titleBuffer->size, &ultimaAssets.titleScreen)) {
      sceneDiskLoader_freeImage(&ultimaAssets.titleScreen);
    }
    free(titleBuffer->data);
    free(titleBuffer);
  }

  Buffer *townBuffer = sceneDiskLoader_readDos33FileByName(disk2, "TWN.PIC");
  if (townBuffer && townBuffer->data) {
    // Decode HGR image
    if (!sceneDiskLoader_decodeHGRImage(townBuffer->data, townBuffer->size, &ultimaAssets.townScreen)) {
      sceneDiskLoader_freeImage(&ultimaAssets.townScreen);
    }
    free(townBuffer->data);
    free(townBuffer);
  }

  Buffer *castleBuffer = sceneDiskLoader_readDos33FileByName(disk2, "CAS.PIC");
  if (castleBuffer && castleBuffer->data) {
    // Decode HGR image
    if (!sceneDiskLoader_decodeHGRImage(castleBuffer->data, castleBuffer->size, &ultimaAssets.castleScreen)) {
      sceneDiskLoader_freeImage(&ultimaAssets.castleScreen);
    }
    free(castleBuffer->data);
    free(castleBuffer);
  }

  // Extract overworld tiles
  Buffer *ultShapesBuffer = sceneDiskLoader_readDos33FileByName(disk1, "ULTSHAPES");
  if (ultShapesBuffer && ultShapesBuffer->data) {
    sceneDiskLoader_decodeUltShapesTiles(ultShapesBuffer->data, ultShapesBuffer->size, &ultimaAssets.overworldTiles);
    free(ultShapesBuffer->data);
    free(ultShapesBuffer);
  }

  // Extract Bterra maps
  for (int i=0;i<OS_BTERRA_COUNT;i++) {
    char name[16];
    snprintf(name, sizeof(name), "BTERRA%d", i);
    Buffer *btBuffer = sceneDiskLoader_readDos33FileByName(disk2, name);
    if (btBuffer && btBuffer->data) {
      sceneDiskLoader_decodeBterraMap(btBuffer->data, btBuffer->size, ultimaAssets.bterraMaps[i]);
      free(btBuffer->data);
      free(btBuffer);
    }
  }

  // Read Strings
  memset(ultimaStrings, 0, sizeof(ultimaStrings));

  // Main Menu
  Buffer *mainMenuBuffer = sceneDiskLoader_readDos33FileByName(disk1, "INIT DISPLAY");
  if (mainMenuBuffer && mainMenuBuffer->data) {
    size_t pos = 0;
    while (pos + 4 <= mainMenuBuffer->size) {
      uint16_t nextAddr = (uint16_t)(mainMenuBuffer->data[pos] | (mainMenuBuffer->data[pos + 1] << 8));
      pos += 4;

      size_t start = pos;
      while (pos < mainMenuBuffer->size && mainMenuBuffer->data[pos] != 0) {
        pos++;
      }

      if (pos > start) {
        sceneDiskLoader_emitQuotedStringsFromAppleBasicLine(mainMenuBuffer->data + start, pos - start);
      }

      if (pos < mainMenuBuffer->size && mainMenuBuffer->data[pos] == 0) {
        pos++;
      }

      if (nextAddr == 0) {
        break;
      }
    }

    free(mainMenuBuffer->data);
    free(mainMenuBuffer);
  }

  // BEVERY
  Buffer *beveryBuffer = sceneDiskLoader_readDos33FileByName(disk2, "BEVERY");
  if (beveryBuffer && beveryBuffer->data) {
    size_t offset = 4;

    char stat[16] = {0};
    memcpy(stat, beveryBuffer->data + offset + 0x1C67, 15);
    sceneDiskLoader_addToUltimaStrings(stat, strlen(stat));

    memcpy(stat, beveryBuffer->data + offset + 0x1C58, 15);
    sceneDiskLoader_addToUltimaStrings(stat, strlen(stat));

    memcpy(stat, beveryBuffer->data + offset + 0x1C49, 15);
    sceneDiskLoader_addToUltimaStrings(stat, strlen(stat));

    memcpy(stat, beveryBuffer->data + offset + 0x1C3A, 15);
    sceneDiskLoader_addToUltimaStrings(stat, strlen(stat));

    memcpy(stat, beveryBuffer->data + offset + 0x1C2B, 15);
    sceneDiskLoader_addToUltimaStrings(stat, strlen(stat));

    memcpy(stat, beveryBuffer->data + offset + 0x1C1C, 15);
    sceneDiskLoader_addToUltimaStrings(stat, strlen(stat));

    char class[5] = {0};
    memcpy(class, beveryBuffer->data + offset + 0x1DFB, 4);
    sceneDiskLoader_addToUltimaStrings(class, strlen(class));

    memcpy(class, beveryBuffer->data + offset + 0x1DF7, 4);
    sceneDiskLoader_addToUltimaStrings(class, strlen(class));

    char race[7] = {0};
    memcpy(race, beveryBuffer->data + offset + 0x13CF, 5);
    sceneDiskLoader_addToUltimaStrings(race, 5);

    memcpy(race, beveryBuffer->data + offset + 0x13CC, 3);
    sceneDiskLoader_addToUltimaStrings(race, 3);

    memcpy(race, beveryBuffer->data + offset + 0x13C7, 5);
    sceneDiskLoader_addToUltimaStrings(race, 5);

    memcpy(race, beveryBuffer->data + offset + 0x13C1, 6);
    sceneDiskLoader_addToUltimaStrings(race, 6);

    char type[8] = {0};
    memcpy(type, beveryBuffer->data + offset + 0x13AD, 7);
    sceneDiskLoader_addToUltimaStrings(type, 7);

    memcpy(type, beveryBuffer->data + offset + 0x13A7, 6);
    sceneDiskLoader_addToUltimaStrings(type, 6);

    memcpy(type, beveryBuffer->data + offset + 0x13A1, 6);
    sceneDiskLoader_addToUltimaStrings(type, 6);

    memcpy(type, beveryBuffer->data + offset + 0x139C, 5);
    sceneDiskLoader_addToUltimaStrings(type, 5);

    free(beveryBuffer->data);
    free(beveryBuffer);
  }

  // Out Move
  Buffer *outMoveBuffer = sceneDiskLoader_readDos33FileByName(disk1, "OUT MOVE");
  if (outMoveBuffer && outMoveBuffer->data) {
    size_t pos = 0;
    while (pos + 4 <= outMoveBuffer->size) {
      uint16_t nextAddr = (uint16_t)(outMoveBuffer->data[pos] | (outMoveBuffer->data[pos + 1] << 8));
      pos += 4;

      size_t start = pos;
      while (pos < outMoveBuffer->size && outMoveBuffer->data[pos] != 0) {
        pos++;
      }

      if (pos > start) {
        sceneDiskLoader_emitQuotedStringsFromAppleBasicLine(outMoveBuffer->data + start, pos - start);
      }

      if (pos < outMoveBuffer->size && outMoveBuffer->data[pos] == 0) {
        pos++;
      }

      if (nextAddr == 0) {
        break;
      }
    }

    free(outMoveBuffer->data);
    free(outMoveBuffer);
  }

  free(disk1);
  free(disk2);

  ultimaAssets.loaded = true;
}

void sceneDiskLoader_init() {
  sceneDiskLoader_verifyUltimaDisks();
  sceneDiskLoader_extractUltimaAssets();

  loaderTime = 2.0f; // Show the loader for 2 seconds before proceeding
}

void sceneDiskLoader_update(float deltaTime) {
  int x = (OS_SCREEN_WIDTH - (strlen(diskMsgText) * OS_FONT_GLYPH_WIDTH)) / 2;
  text_render(&diskMsg, x, OS_SCREEN_HEIGHT / 2);

  if (loaderTime > 0.0f && ultimaAssets.loaded) {
    loaderTime -= deltaTime;
    if (loaderTime <= 0.0f) {
      scene_load(&sceneSplash);
    }
  }
}

void sceneDiskLoader_free() {
  text_free(&diskMsg);
}

void sceneDiskLoader_freeTextures() {
  if (ultimaAssets.loaded) {
    texture_free(ultimaAssets.titleScreen.textureId);
    ultimaAssets.titleScreen.textureId = 0;
    texture_free(ultimaAssets.townScreen.textureId);
    ultimaAssets.townScreen.textureId = 0;
    texture_free(ultimaAssets.castleScreen.textureId);
    ultimaAssets.castleScreen.textureId = 0;
    texture_free(ultimaAssets.overworldTiles.textureId);
    ultimaAssets.overworldTiles.textureId = 0;
  }
}

Scene sceneDiskLoader = {
  .scene_init = sceneDiskLoader_init,
  .scene_update = sceneDiskLoader_update,
  .scene_free = sceneDiskLoader_free
};
