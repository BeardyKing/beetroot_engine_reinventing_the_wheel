#ifndef VK_BEETROOT_WINDOW_H
#define VK_BEETROOT_WINDOW_H

//===api=====================
bool window_is_open();
void window_poll();

//===init & shutdown=========
void window_create();
void window_cleanup();

#endif //VK_BEETROOT_WINDOW_H
