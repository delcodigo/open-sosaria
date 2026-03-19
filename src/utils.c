#include <stdio.h>
#include <stdlib.h>

int file_exists(const char *filename) {
  FILE *file = fopen(filename, "r");

  if (file) {
    fclose(file);
    return 1;
  }

  return 0;
}

float rand01() {
  return (float)rand() / ((float)RAND_MAX + 1.0f);
}