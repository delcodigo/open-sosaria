#ifndef OS_MEMORY_H
#define OS_MEMORY_H

#ifdef DEBUG
#include <stddef.h>
void *osCalloc(size_t count, size_t size, const char* file, int line);
void *osMalloc(size_t size, const char* file, int line);
void *osRealloc(void *ptr, size_t size, const char *file, int line);
void osFree(void *ptr, const char *file, int line);

#define calloc(count, size) osCalloc(count, size, __FILE__, __LINE__)
#define realloc(ptr, size) osRealloc(ptr, size, __FILE__, __LINE__)
#define malloc(size) osMalloc(size, __FILE__, __LINE__)
#define free(size) osFree(size, __FILE__, __LINE__)
#endif

#endif

void printLeftPointers();