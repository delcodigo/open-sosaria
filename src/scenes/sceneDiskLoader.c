#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <math.h>
#include "engine/text.h"
#include "engine/texture.h"
#include "sceneDiskLoader.h"
#include "maths/matrix4.h"
#include "utils.h"
#include "memory.h"
#include "config.h"
#include "sceneSplash.h"
#include "data/enemy.h"
#include "data/bevery.h"

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

static uint8_t colorBlack[3] = {0, 0, 0};
static uint8_t colorBlue[3] = {0, 146, 255};
static uint8_t colorPurple[3] = {146, 0, 255};
static uint8_t colorGreen[3] = {36, 182, 0};
static uint8_t colorWhite[3] = {255, 255, 255};
static uint8_t colorOrange[3] = {255, 86, 0};

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

static void sceneDiskLoader_fillHGRGaps(UltimaImage *outImage) {
  int width = outImage->width;
  int height = outImage->height;

  for (int y=0;y<height;y++) {
    for (int x=1;x<width-1;x++) {
      uint32_t index = (y * width + x) * 4;
      uint32_t nextIndex = (y * width + x + 1) * 4;
      uint32_t prevIndex = (y * width + x - 1) * 4;

      bool isCurrBlack = (outImage->data[index+0] == colorBlack[0] && outImage->data[index+1] == colorBlack[1] && outImage->data[index+2] == colorBlack[2]);
      bool neighborsSameColor = (
        outImage->data[prevIndex+0] == outImage->data[nextIndex+0] &&
        outImage->data[prevIndex+1] == outImage->data[nextIndex+1] &&
        outImage->data[prevIndex+2] == outImage->data[nextIndex+2]
      );
      bool neighborIsLit = (outImage->data[prevIndex+0] != 0 || outImage->data[prevIndex+1] != 0 || outImage->data[prevIndex+2] != 0);
      bool isNeighborWhite = (outImage->data[prevIndex+0] == colorWhite[0] && outImage->data[prevIndex+1] == colorWhite[1] && outImage->data[prevIndex+2] == colorWhite[2]);

      if (isCurrBlack && neighborsSameColor && neighborIsLit) {
        int r, g, b;

        if (!isNeighborWhite) {
          r = outImage->data[prevIndex+0];
          g = outImage->data[prevIndex+1];
          b = outImage->data[prevIndex+2];
        } else if ((x & 1) == 0) {
          r = colorGreen[0]; g = colorGreen[1]; b = colorGreen[2];
        } else {
          r = colorPurple[0]; g = colorPurple[1]; b = colorPurple[2];
        }

        outImage->data[index + 0] = r;
        outImage->data[index + 1] = g;
        outImage->data[index + 2] = b;
        outImage->data[index + 3] = 255;
      }
    }
  }
}

static bool sceneDiskLoader_decodeHGRImage(const uint8_t *dataRaw, uint32_t sizeRaw, UltimaImage *outImage) {
  if (!dataRaw || !outImage) { return false; }

  uint32_t size = 0;
  const uint8_t *data = sceneDiskLoader_maybeStripBloadHeader(dataRaw, sizeRaw, &size);
  if (!data || size < 8184) { return false; }

  const int originalWidth = 280;
  const int originalHeight = 192;

  *outImage = sceneDiskLoader_createImage(originalWidth, originalHeight, colorBlack[0], colorBlack[1], colorBlack[2]);
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
          r = colorBlack[0]; g = colorBlack[1]; b = colorBlack[2];
        } else if (left || right) {
          // adjacent lit pixels trend toward white
          r = colorWhite[0]; g = colorWhite[1]; b = colorWhite[2];
        } else {
          // isolated pixel -> artifact color
          int odd = x & 1;
          int ph = phase[x];

          // Apple II NTSC artifact colors:
          // ph=0: even -> purple, odd -> green
          // ph=1: even -> blue,   odd -> orange
          if (ph == 0) {
            if (odd) {
              r = colorGreen[0]; g = colorGreen[1]; b = colorGreen[2];
            } else {
              r = colorPurple[0]; g = colorPurple[1]; b = colorPurple[2];
            }
          } else {
            if (odd) {
              r = colorOrange[0]; g = colorOrange[1]; b = colorOrange[2];
            } else {
              r = colorBlue[0]; g = colorBlue[1]; b = colorBlue[2];
            }
          }
        }

        sceneDiskLoader_setPixel(outImage, x, y, r, g, b);
    }
  }

  sceneDiskLoader_fillHGRGaps(outImage);
  outImage->textureId = texture_load(outImage->width, outImage->height, outImage->data);
  sceneDiskLoader_freeImage(outImage);

  return true;
}

static void sceneDiskLoader_emitQuotedStringsFromAppleBasicLine(const unsigned char *lineData, size_t lineLength) {
  size_t i = 0;
  bool shouldAppend = false;

  while (i<lineLength) {
    if (lineData[i] == '"') {
      size_t start = i + 1;
      bool isEmpty = true;
      
      while (lineData[start] == '\a') {
        i++;
        start++;
        isEmpty = false;
      }

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
      } else if (!isEmpty) {
        ultimaStrings[ultimaStringCount++][0] = '\0';
      }

      i = end + 1;
      continue;
    }
    
    i++;
  }
}

static void sceneDiskLoader_hgrColorForPixel(bool on, bool hi, bool parityEven, bool adjacentOn, uint8_t *r, uint8_t *g, uint8_t *b) {
  if (!on) {
    *r = colorBlack[0]; *g = colorBlack[1]; *b = colorBlack[2];
  } else if (adjacentOn) {
    *r = colorWhite[0]; *g = colorWhite[1]; *b = colorWhite[2];
  } else if (hi) {
    if (parityEven) {
      *r = colorBlue[0]; *g = colorBlue[1]; *b = colorBlue[2];
    } else {
      *r = colorOrange[0]; *g = colorOrange[1]; *b = colorOrange[2];
    }
  } else {
    if (parityEven) {
      *r = colorPurple[0]; *g = colorPurple[1]; *b = colorPurple[2];
    } else {
      *r = colorGreen[0]; *g = colorGreen[1]; *b = colorGreen[2];
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

  *outImage = sceneDiskLoader_createImage(originalWidth, originalHeight, colorBlack[0], colorBlack[1], colorBlack[2]);
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

  sceneDiskLoader_fillHGRGaps(outImage);

  outImage->textureId = texture_load(outImage->width, outImage->height, outImage->data);
  sceneDiskLoader_freeImage(outImage);
  outImage->width = originalWidth;
  outImage->height = originalHeight;
}

static void sceneDiskLoader_decodeBterraMap(const uint8_t *dataRaw, uint32_t sizeRaw, uint8_t map[OS_BTERRA_MAP_WIDTH][OS_BTERRA_MAP_HEIGHT]) {
  if (!dataRaw || !map) { return; }

  uint32_t size = 0;
  const uint8_t *data = sceneDiskLoader_maybeStripBloadHeader(dataRaw, sizeRaw, &size);
  if (!data || size < 64 * 64) { return; }

  for (int y=0;y<64;y++) {
    for (int x=0;x<64;x++) {
      int idx = y * 64 + x;
      map[y][x] = data[idx];
    }
  }
}

static float sceneDiskLoader_decodeApplesoftFloat(const uint8_t *data) {
  uint8_t exponent = data[0];
  if (exponent == 0) return 0.0f;

  int sign = (data[1] & 0x80) ? -1 : 1;

  uint32_t mantissa =
    ((uint32_t) (data[1] & 0x7F) << 24) |
    ((uint32_t) data[2] << 16) |
    ((uint32_t) data[3] << 8) |
    (uint32_t) data[4];

  float frac = 1.0f + ((float)mantissa / 2147483648.0f); // 2^31
  int exp2 = (int) exponent - 129;

  return sign * ldexpf(frac, exp2);
}

static void sceneDiskLoader_decodeEnemiesTable(const uint8_t *data) {
  uint16_t enemiesCount = data[0x496] << 8 | data[0x497];

  enemyDefinitions = calloc(enemiesCount, sizeof(EnemyDefinition));
  if (!enemyDefinitions) { return; }

  int offset = 0x498;
  int nameOffset = 0x405;
  int stride = 5;
  for (int i=0;i<enemiesCount;i++) {
    float group = sceneDiskLoader_decodeApplesoftFloat(data + offset + (0 * enemiesCount + i) * stride);
    float rank = sceneDiskLoader_decodeApplesoftFloat(data + offset + (1 * enemiesCount + i) * stride);
    float hp = sceneDiskLoader_decodeApplesoftFloat(data + offset + (2 * enemiesCount + i) * stride);

    int nameLength = data[nameOffset + i * 3];
    int nameAddress = ((int)data[nameOffset + i * 3 + 1] | ((int)data[nameOffset + i * 3 + 2] << 8)) - 0x7800;
    char name[17] = {0};
    memcpy(name, data + nameAddress, nameLength);

    enemy_define(i, name, group, rank, hp);
  }

  memcpy(enemyDefinitions[12].name, ultimaStrings[14], strlen(ultimaStrings[14]) + 1);
  memcpy(enemyDefinitions[6].name, ultimaStrings[15], strlen(ultimaStrings[15]) + 1);
  memcpy(enemyDefinitions[7].name, ultimaStrings[16], strlen(ultimaStrings[16]) + 1);
  memcpy(enemyDefinitions[10].name, ultimaStrings[17], strlen(ultimaStrings[17]) + 1);
  memcpy(enemyDefinitions[19].name, ultimaStrings[18], strlen(ultimaStrings[18]) + 1);
  memcpy(enemyDefinitions[20].name, ultimaStrings[19], strlen(ultimaStrings[19]) + 1);
  memcpy(enemyDefinitions[30].name, ultimaStrings[21], strlen(ultimaStrings[21]) + 1);
  memcpy(enemyDefinitions[33].name, ultimaStrings[22], strlen(ultimaStrings[22]) + 1);
  memcpy(enemyDefinitions[39].name, ultimaStrings[23], strlen(ultimaStrings[23]) + 1);
  memcpy(enemyDefinitions[41].name, ultimaStrings[24], strlen(ultimaStrings[24]) + 1);
  memcpy(enemyDefinitions[32].name, ultimaStrings[25], strlen(ultimaStrings[25]) + 1);
}

static void sceneDiskLoader_decodeUltimaStrings(const uint8_t *data, size_t index, char ***variable, int *variableSize) {
  int size = data[index] << 8 | data[index + 1];

  *variableSize = size;
  *variable = calloc(size, sizeof(char *));
  if (!*variable) { return; }

  int offset = index + 2;
  for (int i=0;i<size;i++) {
    int strLen = data[offset];
    int address = (data[offset + 1] | data[offset + 2] << 8) - 0x7800;

    (*variable)[i] = calloc(strLen + 1, sizeof(char));
    if (!(*variable)[i]) { return; }
    
    memcpy((*variable)[i], data + address, strLen);
    (*variable)[i][strLen] = '\0';
    offset += 3;
  }
}  

static void sceneDiskLoader_decodeDungeonTable(const uint8_t *data, size_t index) {
  index += 4;
  for (int x=0;x<OS_DUNGEON_TABLE_WIDTH;x++) {
    for (int y=0;y<OS_DUNGEON_TABLE_HEIGHT;y++) {
      int value = data[index] << 8 | data[index + 1];
      dungeonTable[y][x] = value;
      index += 2;
    }
  }
}

static size_t sceneDiskLoader_findVariableOffset(const uint8_t *data, size_t dataSize, const char *varName) {
  if (!data || !varName || !varName[0]) { return 0; }

  char first = 0, second = 0;
  int letters = 0;

  for (const char *p=varName;*p;++p) {
    if (*p == '$'|| *p == '%') { continue; }
    if (isalpha((unsigned char)*p)) {
      if (letters == 0){ first = (char)toupper((unsigned char)*p); } else
      if (letters == 1){ second = (char)toupper((unsigned char)*p); }
      letters++;
      if (letters > 2) { return 0; }
    } else {
      return 0;
    }
  }

  if (letters == 0) { return 0; }

  uint8_t b0 = (uint8_t) first;
  uint8_t b1 = (letters == 1) ? 0x80 : (uint8_t)(second | 0x80);

  for (size_t i=0;i+1<dataSize;i++) {
    if ((data[i] & 0x7F) == b0 && data[i+1] == b1) {
      return i;
    }
  }

  return 0;
}

static void sceneDiskLoader_extractHGRImage(uint8_t *disk, const char *fileName, UltimaImage *outImage) {
  Buffer *imageBuffer = sceneDiskLoader_readDos33FileByName(disk, fileName);
  if (imageBuffer && imageBuffer->data) {
    // Decode HGR image
    if (!sceneDiskLoader_decodeHGRImage(imageBuffer->data, imageBuffer->size, outImage)) {
      sceneDiskLoader_freeImage(outImage);
    }
    free(imageBuffer->data);
    free(imageBuffer);
  }
}

static void sceneDiskLoader_extractBasicStrings(uint8_t *disk, const char *fileName) {
  Buffer *fileBuffer = sceneDiskLoader_readDos33FileByName(disk, fileName);
  if (fileBuffer && fileBuffer->data) {
    size_t pos = 0;
    while (pos + 4 <= fileBuffer->size) {
      uint16_t nextAddr = (uint16_t)(fileBuffer->data[pos] | (fileBuffer->data[pos + 1] << 8));
      pos += 4;

      size_t start = pos;
      while (pos < fileBuffer->size && fileBuffer->data[pos] != 0) {
        pos++;
      }

      if (pos > start) {
        sceneDiskLoader_emitQuotedStringsFromAppleBasicLine(fileBuffer->data + start, pos - start);
      }

      if (pos < fileBuffer->size && fileBuffer->data[pos] == 0) {
        pos++;
      }

      if (nextAddr == 0) {
        break;
      }
    }

    free(fileBuffer->data);
    free(fileBuffer);
  }
}

bool sceneDiskLoader_parseShapeTable(const uint8_t *dataRaw, uint32_t sizeRaw, ShapeTable *table) {
  if (!dataRaw || !table) { return false; }

  uint32_t size = 0;
  const uint8_t *data = sceneDiskLoader_maybeStripBloadHeader(dataRaw, sizeRaw, &size);
  if (!data || size < 8) { return false; }

  uint16_t count = (uint16_t)(data[0] | (data[1] << 8));
  if (count < 1 || count > 255) { return false; }

  // Try mode with count+1 pointers first, then count pointers
  for (int j=1;j>=0;j--) {
    int ptrWords = count + j;
    if (2 + ptrWords * 2 <= (int)size) {
      uint16_t *ptr = (uint16_t *)malloc(ptrWords * sizeof(uint16_t));
      if (!ptr) { continue; }

      for (int i=0;i<ptrWords;i++) {
        ptr[i] = (uint16_t)(data[2 + i * 2] | (data[2 + i * 2 + 1] << 8));
      }

      bool valid = true;
      for (int i=0;i<ptrWords;i++) {
        if (ptr[i] > size) { valid = false; break; }
        if (i > 0 && ptr[i] < ptr[i-1]) { valid = false; break; }
      }

      if (valid) {
        if (ptrWords == count + 1) {
          uint16_t *starts = (uint16_t *)malloc(count * sizeof(uint16_t));
          uint16_t *ends = (uint16_t *)malloc(count * sizeof(uint16_t));
          if (!starts || !ends) {
            free(starts);
            free(ends);
            free(ptr);
            continue;
          }

          memcpy(starts, ptr, count * sizeof(uint16_t));
          memcpy(ends, ptr + 1, count * sizeof(uint16_t));

          bool rangesValid = true;
          for (int i=0;i<count;i++) {
            if (starts[i] >= ends[i] || ends[i] > size) {
              rangesValid = false;
              break;
            }
          }

          if (!rangesValid) {
            free(starts);
            free(ends);
            free(ptr);
            continue;
          }

          table->count = count;
          table->starts = starts;
          table->ends = ends;
          table->data = data;
          table->data_size = size;
          free(ptr);
          return true;
        }

        if (ptrWords == count) {
          uint16_t *ends = (uint16_t *)malloc(count * sizeof(uint16_t));
          if (!ends) {
            free(ptr);
            continue;
          }

          bool rangesValid = true;
          
          for (int i=0;i<count;i++) {
            uint16_t s = ptr[i];
            uint16_t limit = i + 1 < count ? ptr[i + 1] : (uint16_t)size;
            uint16_t e = limit;

            if (s >= limit || limit > size) {
              rangesValid = false;
              break;
            }

            for (uint16_t p=s;p<limit;p++) {
              if (data[p] == 0) {
                e = p + 1;
                break;
              }
            }

            if (e <= s || e > size) {
              rangesValid = false;
              break;
            }

            ends[i] = e;
          }

          if (!rangesValid) {
            free(ends);
            free(ptr);
            continue;
          }

          table->count = count;
          table->starts = ptr;
          table->ends = ends;
          table->data = data;
          table->data_size = size;
          return true;
        }
      }

      free(ptr);
    }
  }

  return false;
}

static void sceneDiskLoader_colorHGRShapeTable(int width, int height, UltimaImage *outImage) {
  for (int y=0;y<height;y++) {
    for (int x=0;x<width;x++) {
      uint32_t index = (y * width + x) * 4;
      
      bool isLit = outImage->data[index] != 0 || outImage->data[index + 1] != 0 || outImage->data[index + 2] != 0;
      if (!isLit) { continue; }

      bool leftLit = false;
      bool rightLit = false;
      
      if (x > 0) { leftLit = outImage->data[index - 4] != 0 || outImage->data[index - 3] != 0 || outImage->data[index - 2] != 0; }
      if (x < width - 1) { rightLit = outImage->data[index + 4] != 0 || outImage->data[index + 5] != 0 || outImage->data[index + 6] != 0; }

      if (!leftLit && !rightLit) {
        // isolated lit pixel -> NTSC artifact color (shape tables have no hi-bit, default ph=0)
        if ((x & 1) == 0) {
          outImage->data[index + 0] = colorPurple[0];
          outImage->data[index + 1] = colorPurple[1];
          outImage->data[index + 2] = colorPurple[2];
        } else {
          outImage->data[index + 0] = colorGreen[0];
          outImage->data[index + 1] = colorGreen[1];
          outImage->data[index + 2] = colorGreen[2];
        }
      }
    }
  }

  sceneDiskLoader_fillHGRGaps(outImage);
}

static void sceneDiskLoader_plotShapeTable(uint8_t byte, int *x, int *y, uint8_t *color, UltimaImage *outImage) {
  // Direction vectors: up, right, down, left
  const int dirs[4][2] = {
    {0, -1},
    {1, 0},
    {0, 1},
    {-1, 0}
  };

  // Apple II shape table format
  int aDir = byte & 0x03;
  bool aPlot = ((byte>>2) & 0x01) != 0;
  int bDir = (byte >> 3) & 0x03;
  bool bPlot = ((byte >> 5) & 0x01) != 0;
  int cDir = (byte >> 6) & 0x03;

  if (aPlot) { sceneDiskLoader_setPixel(outImage, *x, *y, color[0], color[1], color[2]); }
  *x += dirs[aDir][0];
  *y += dirs[aDir][1];

  if (!(bDir == 0 && !bPlot && cDir == 0)) {
    if (bPlot) { sceneDiskLoader_setPixel(outImage, *x, *y, color[0], color[1], color[2]); }
    *x += dirs[bDir][0];
    *y += dirs[bDir][1];
  }

  if (cDir != 0) {
    *x += dirs[cDir][0];
    *y += dirs[cDir][1];
  }
}

static bool sceneDiskLoader_renderEnemiesShapeTable(const ShapeTable *table, UltimaImage *outImage, int tileWidth, int tileHeight) {
  if (!table || !outImage) { return false; }

  const int cols = 8;
  const int rows = (int) ceil((float)table->count / (float)cols);
  const int originalWidth = cols * tileWidth;
  const int originalHeight = rows * tileHeight;

  *outImage = sceneDiskLoader_createImage(originalWidth, originalHeight, 255, 0, 0);
  if (!outImage->data) { return false; }

  for (int s=table->count-1;s>=0;s--) {
    int sind = (s >= table->count / 2) ? s - table->count / 2 : s;
    int cx = (sind % cols) * tileWidth;
    int cy = (sind / cols) * tileHeight;
    int x = cx;
    int y = cy;

    uint16_t start = table->starts[s];
    uint16_t end = table->ends[s];

    for (uint16_t i=start;i<end && i<table->data_size;i++) {
      uint8_t byte = table->data[i];
      if (byte == 0) { continue; }

      uint8_t *color = (s >= table->count / 2) ? colorBlack : colorWhite;

      sceneDiskLoader_plotShapeTable(byte, &x, &y, color, outImage);
    }
  }

  for (int y=0;y<originalHeight;y++) {
    for (int x=0;x<originalWidth;x++) {
      uint32_t index = (y * originalWidth + x) * 4;
      if (outImage->data[index] == 255 && outImage->data[index + 1] == 0 && outImage->data[index + 2] == 0) {
        outImage->data[index + 0] = 0;
        outImage->data[index + 1] = 0;
        outImage->data[index + 2] = 0;
        outImage->data[index + 3] = 0;
        continue;
      }
    }
  }

  sceneDiskLoader_colorHGRShapeTable(originalWidth, originalHeight, outImage);

  outImage->textureId = texture_load(outImage->width, outImage->height, outImage->data);
  sceneDiskLoader_freeImage(outImage);

  return true;
}

static bool sceneDiskLoader_renderShapeTable(const ShapeTable *table, UltimaImage *outImage, int tileWidth, int tileHeight) {
  if (!table || !outImage) { return false; }

  const int cols = 8;
  const int rows = (int) ceil((float)table->count / (float)cols);
  const int originalWidth = cols * tileWidth;
  const int originalHeight = rows * tileHeight;

  *outImage = sceneDiskLoader_createImage(originalWidth, originalHeight, 0, 0, 0);
  if (!outImage->data) { return false; }

  for (int s=0;s<table->count;s++) {
    int cx = (s % cols) * tileWidth;
    int cy = (s / cols) * tileHeight;
    int x = cx;
    int y = cy;

    uint16_t start = table->starts[s];
    uint16_t end = table->ends[s];

    for (uint16_t i=start;i<end && i<table->data_size;i++) {
      uint8_t byte = table->data[i];
      if (byte == 0) { continue; }

      sceneDiskLoader_plotShapeTable(byte, &x, &y, colorWhite, outImage);
    }
  }

  sceneDiskLoader_colorHGRShapeTable(originalWidth, originalHeight, outImage);

  outImage->textureId = texture_load(outImage->width, outImage->height, outImage->data);
  sceneDiskLoader_freeImage(outImage);

  return true;
}

static void sceneDiskLoader_freeShapeTable(ShapeTable *table) {
  if (table) {
    free(table->starts);
    free(table->ends);
    table->starts = NULL;
    table->ends = NULL;
    table->count = 0;
    table->data = NULL;
    table->data_size = 0;
  }
}

static void sceneDiskLoader_loadCollisionsMap(uint8_t *disk, char* fileName, uint8_t map[OS_TOWN_CASTLE_SIZE_HEIGHT][OS_TOWN_CASTLE_SIZE_WIDTH]) {
  Buffer *fileBuffer = sceneDiskLoader_readDos33FileByName(disk, fileName);
  if (fileBuffer && fileBuffer->data) {
    uint32_t size = 0;
    const uint8_t *data = sceneDiskLoader_maybeStripBloadHeader(fileBuffer->data, fileBuffer->size, &size);

    if (!data || size < OS_TOWN_CASTLE_SIZE_WIDTH * OS_TOWN_CASTLE_SIZE_HEIGHT) {
      free(fileBuffer->data);
      free(fileBuffer);
      return;
    }

    uint8_t *ptr = (uint8_t *)data;
    for (int x=0;x<OS_TOWN_CASTLE_SIZE_WIDTH;x++) {
      for (int y=0;y<OS_TOWN_CASTLE_SIZE_HEIGHT;y++) {
        map[y][x] = *ptr++;
      }
    }
    
    free(fileBuffer->data);
    free(fileBuffer);
  }
}

static int sceneDiskLoader_verifyUltimaDisks() {
  if (!file_exists("program.dsk")) {
    strcpy(diskMsgText, "'program.dsk' not found!");
    text_create(&diskMsg, diskMsgText);
    return 0;
  }

  if (!file_exists("player.dsk")) {
    strcpy(diskMsgText, "'player.dsk' not found!");
    text_create(&diskMsg, diskMsgText);
    return 0;
  }

  uint8_t *disk1 = (uint8_t *)malloc(DISK_SIZE);
  uint8_t *disk2 = (uint8_t *)malloc(DISK_SIZE);

  if (!disk1 || !disk2) {
    strcpy(diskMsgText, "Failed to allocate memory for disks!");
    text_create(&diskMsg, diskMsgText);
    free(disk1);
    free(disk2);
    return 0;
  }

  FILE *file1 = fopen("program.dsk", "rb");
  FILE *file2 = fopen("player.dsk", "rb");

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

  strcpy(diskMsgText, "--OPEN SOSARIA--");
  text_create(&diskMsg, diskMsgText);
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

  FILE *file1 = fopen("program.dsk", "rb");
  FILE *file2 = fopen("player.dsk", "rb");

  fread(disk1, 1, DISK_SIZE, file1);
  fread(disk2, 1, DISK_SIZE, file2);
  fclose(file1);
  fclose(file2);

  // Extract Apple II HGR Images
  sceneDiskLoader_extractHGRImage(disk1, "PIC.ULTIMATUM", &ultimaAssets.titleScreen);
  sceneDiskLoader_extractHGRImage(disk2, "TWN.PIC", &ultimaAssets.townScreen);
  sceneDiskLoader_extractHGRImage(disk2, "CAS.PIC", &ultimaAssets.castleScreen);

  // Extract overworld tiles
  Buffer *ultShapesBuffer = sceneDiskLoader_readDos33FileByName(disk1, "ULTSHAPES");
  if (ultShapesBuffer && ultShapesBuffer->data) {
    sceneDiskLoader_decodeUltShapesTiles(ultShapesBuffer->data, ultShapesBuffer->size, &ultimaAssets.overworldTiles);
    free(ultShapesBuffer->data);
    free(ultShapesBuffer);
  }

  // Extract enemy sprites
  Buffer *outShapesBuffer = sceneDiskLoader_readDos33FileByName(disk1, "OUT.SHAPES");
  if (outShapesBuffer && outShapesBuffer->data) {
    ShapeTable table = {0};
    if (sceneDiskLoader_parseShapeTable(outShapesBuffer->data, outShapesBuffer->size, &table)) {
      sceneDiskLoader_renderEnemiesShapeTable(&table, &ultimaAssets.enemySprites, OS_ENEMY_SPRITE_WIDTH, OS_ENEMY_SPRITE_HEIGHT);
      sceneDiskLoader_freeShapeTable(&table);
    }

    free(outShapesBuffer->data);
    free(outShapesBuffer);
  }

  // Extract town/castle sprites
  Buffer *twnCasShapesBuffer = sceneDiskLoader_readDos33FileByName(disk1, "TWN.CAS.SHAPES");
  if (twnCasShapesBuffer && twnCasShapesBuffer->data) {
    ShapeTable table = {0};
    if (sceneDiskLoader_parseShapeTable(twnCasShapesBuffer->data, twnCasShapesBuffer->size, &table)) {
      sceneDiskLoader_renderShapeTable(&table, &ultimaAssets.townCastleSprites, OS_TOWN_CASTLE_SPRITE_WIDTH, OS_TOWN_CASTLE_SPRITE_HEIGHT);
      sceneDiskLoader_freeShapeTable(&table);
    }

    free(twnCasShapesBuffer->data);
    free(twnCasShapesBuffer);
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

  // BASIC files strings
  sceneDiskLoader_extractBasicStrings(disk1, "INIT DISPLAY");
  sceneDiskLoader_extractBasicStrings(disk1, "OUT MOVE");
  sceneDiskLoader_extractBasicStrings(disk1, "TWN MOVE");
  sceneDiskLoader_extractBasicStrings(disk1, "CAS MOVE");

  // Load town collisions map
  sceneDiskLoader_loadCollisionsMap(disk2, "BTWN", ultimaAssets.townCollisionMap);

  // Load castle collisions map
  sceneDiskLoader_loadCollisionsMap(disk2, "BCAS", ultimaAssets.castleCollisionMap);

  // BEVERY
  Buffer *beveryBuffer = sceneDiskLoader_readDos33FileByName(disk2, "BEVERY");
  if (beveryBuffer && beveryBuffer->data) {
    uint32_t size = 0;
    const uint8_t *data = sceneDiskLoader_maybeStripBloadHeader(beveryBuffer->data, beveryBuffer->size, &size);
    int variableHeaderCount = 5; // Skip the first 5 variable headers until we reach the size of the array

    // Stats
    size_t address = sceneDiskLoader_findVariableOffset(data, size, "C$");
    sceneDiskLoader_decodeUltimaStrings(data, address + variableHeaderCount, &statsNames, &statsNamesSize);

    // Races
    address = sceneDiskLoader_findVariableOffset(data, size, "RA$");
    sceneDiskLoader_decodeUltimaStrings(data, address + variableHeaderCount, &racesNames, &racesNamesSize);

    // Types
    address = sceneDiskLoader_findVariableOffset(data, size, "TY$");
    sceneDiskLoader_decodeUltimaStrings(data, address + variableHeaderCount, &typesNames, &typesNamesSize);

    // Armors
    address = sceneDiskLoader_findVariableOffset(data, size, "AR$");
    sceneDiskLoader_decodeUltimaStrings(data, address + variableHeaderCount, &armorNames, &armorNamesSize);

    // Vehicles
    address = sceneDiskLoader_findVariableOffset(data, size, "VE$");
    sceneDiskLoader_decodeUltimaStrings(data, address + variableHeaderCount, &vehicleNames, &vehicleNamesSize);

    // Weapons
    address = sceneDiskLoader_findVariableOffset(data, size, "PW$");
    sceneDiskLoader_decodeUltimaStrings(data, address + variableHeaderCount, &weaponNames, &weaponNamesSize);

    // Spells
    address = sceneDiskLoader_findVariableOffset(data, size, "SP$");
    sceneDiskLoader_decodeUltimaStrings(data, address + variableHeaderCount, &spellNames, &spellNamesSize);
    memcpy(spellNames[3], ultimaStrings[20], strlen(ultimaStrings[20]) + 1);

    // Places
    address = sceneDiskLoader_findVariableOffset(data, size, "TD$");
    sceneDiskLoader_decodeUltimaStrings(data, address + variableHeaderCount, &placesNames, &placesNamesSize);
    memcpy(placesNames[44], ultimaStrings[26], strlen(ultimaStrings[26]) + 1);

    address = sceneDiskLoader_findVariableOffset(data, size, "PE%");
    sceneDiskLoader_decodeDungeonTable(data, address + variableHeaderCount);

    sceneDiskLoader_decodeEnemiesTable(data);

    free(beveryBuffer->data);
    free(beveryBuffer);
  }

  free(disk1);
  free(disk2);

  ultimaAssets.blackSprite = sceneDiskLoader_createImage(1, 1, 0, 0, 0);
  ultimaAssets.blackSprite.textureId = texture_load(ultimaAssets.blackSprite.width, ultimaAssets.blackSprite.height, ultimaAssets.blackSprite.data);
  free(ultimaAssets.blackSprite.data);

  ultimaAssets.loaded = true;
}

void sceneDiskLoader_init() {
  if (sceneDiskLoader_verifyUltimaDisks()) {
    sceneDiskLoader_extractUltimaAssets();
  }

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
    texture_free(ultimaAssets.enemySprites.textureId);
    ultimaAssets.enemySprites.textureId = 0;
    texture_free(ultimaAssets.townCastleSprites.textureId);
    ultimaAssets.townCastleSprites.textureId = 0;
    texture_free(ultimaAssets.blackSprite.textureId);
    ultimaAssets.blackSprite.textureId = 0;
  }
}

Scene sceneDiskLoader = {
  .scene_init = sceneDiskLoader_init,
  .scene_update = sceneDiskLoader_update,
  .scene_free = sceneDiskLoader_free
};
