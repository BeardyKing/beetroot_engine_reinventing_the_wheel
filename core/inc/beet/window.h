#ifndef VK_BEETROOT_WINDOW_H
#define VK_BEETROOT_WINDOW_H

#include <beet/types.h>

//===api=====================
bool window_is_open();
void window_update();
void window_set_cursor(CursorState state);
void window_set_cursor_lock_position(vec2i lockPos);
bool window_is_cursor_over_window();


//===init & shutdown=========
void window_create();
void window_cleanup();

#endif //VK_BEETROOT_WINDOW_H
