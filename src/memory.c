#include <stdlib.h>
#include <stdio.h>

#ifdef DEBUG
typedef struct {
    void *ptr;
    size_t size;
} Allocation;

#define MAX_ALLOCATIONS 1024
static Allocation allocations[MAX_ALLOCATIONS];
static size_t allocationCount = 0;

void *osCalloc(size_t count, size_t size, const char* file, int line) {
  void *ptr = calloc(count, size);
  if (ptr) {
    allocations[allocationCount++] = (Allocation){ ptr, size };
    printf("Allocated %zu zero initialized bytes at %s:%d, address: %p\n", size, file, line, ptr);
  } else {
    printf("Failed to allocate memory at %s:%d\n", file, line);
  }

  return ptr;
}

void *osMalloc(size_t size, const char* file, int line) {
  void *ptr = malloc(size);
  if (ptr) {
    allocations[allocationCount++] = (Allocation){ ptr, size };
    printf("Allocated %zu bytes at %s:%d, address: %p\n", size, file, line, ptr);
  } else {
    printf("Failed to allocate memory at %s:%d\n", file, line);
  }

  return ptr;
}

void *osRealloc(void *ptr, size_t size, const char *file, int line) {
  if (ptr) {
    for (size_t i=0;i<allocationCount;i++) {
      if (allocations[i].ptr == ptr) {
        printf("Reallocated memory at %s:%d, address: %p\n", file, line, ptr);
        allocations[i].ptr = realloc(ptr, size);
        return allocations[i].ptr;
      }
    }
    fprintf(stderr, "Attempted to reallocate untracked memory at %s:%d, address: %p\n", file, line, ptr);
  } else {
    fprintf(stderr, "Attempted to reallocate NULL pointer at %s:%d\n", file, line);
  }
  return NULL;
}

void osFree(void *ptr, const char *file, int line) {
  if (ptr) {
    for (size_t i=0;i<allocationCount;i++) {
      if (allocations[i].ptr == ptr) {
        printf("Freed memory at %s:%d, address: %p\n", file, line, ptr);
        for (size_t j=i;j<allocationCount-1;j++) {
          allocations[j] = allocations[j+1];
        }
        allocations[allocationCount - 1].ptr = 0;
        allocations[allocationCount - 1].size = 0;
        allocationCount -= 1;
        free(ptr);
        return;
      }
    }
    fprintf(stderr, "Attempted to free untracked memory at %s:%d, address: %p\n", file, line, ptr);
  } else {
    fprintf(stderr, "Attempted to free NULL pointer at %s:%d\n", file, line);
  }
}

void printLeftPointers() {
  printf("Remaining pointers: %ld\n", allocationCount);
  for (size_t i = 0; i < allocationCount; i++) {
    printf("  Address: %p, Size: %zu\n", allocations[i].ptr, allocations[i].size);
  }
}

#else
void printLeftPointers() {}
#endif