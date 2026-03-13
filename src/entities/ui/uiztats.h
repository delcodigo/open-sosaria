#ifndef OS_UI_STATS_H
#define OS_UI_STATS_H

#include "engine/text.h"

extern bool ztatsActive;

void uiZtats_init();
void uiZtats_update(float deltaTime);
void uiZtats_free();

#endif