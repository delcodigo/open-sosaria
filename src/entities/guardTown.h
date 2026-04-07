#ifndef OS_GUARD_TOWN_H
#define OS_GUARD_TOWN_H

#define OS_GUARD_TOWN_COUNT 6

typedef struct {
  int x;
  int y;
  int hp;
} GuardTown;

extern GuardTown guardTowns[OS_GUARD_TOWN_COUNT];

void guardTown_init();
void guardCastle_init();
void guardTown_update(GuardTown *guard);
void guardTown_render(float *viewMatrix);
void guardTown_free();

#endif