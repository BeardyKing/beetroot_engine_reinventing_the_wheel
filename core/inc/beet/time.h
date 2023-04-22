#ifndef VK_BEETROOT_TIME_H
#define VK_BEETROOT_TIME_H

#include <beet/types.h>

//===api=====================
void time_tick();
double time_delta();
double time_current();
uint32_t time_frame_count();

//===init & shutdown=========
void time_create();
void time_cleanup();

#endif //VK_BEETROOT_TIME_H
