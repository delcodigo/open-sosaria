#ifndef OS_VEHICLE_OVERWORLD_H
#define OS_VEHICLE_OVERWORLD_H

#include "engine/geometry.h"
#include "config.h"

extern unsigned char vehiclesMap[OS_BTERRA_MAP_WIDTH * 2][OS_BTERRA_MAP_HEIGHT * 2];

void vehicleOverworld_init();
void vehicleOverworld_free();
void vehicleOverworld_render(float *viewMatrix);

#endif