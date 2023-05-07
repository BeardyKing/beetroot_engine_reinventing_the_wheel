#ifndef BEETROOT_WINDOW_H
#define BEETROOT_WINDOW_H

#include <math/vec2.h>
#include <core/input_types.h>
#include <vulkan/vulkan_core.h>


//===api=====================
bool window_is_open();
void window_update();
void window_set_cursor(CursorState state);
void window_set_cursor_lock_position(vec2i lockPos);
bool window_is_cursor_over_window();

//===vulkan api==============
void window_create_render_surface(const VkInstance* instance, VkSurfaceKHR* out_surface);

//===init & shutdown=========
void window_create();
void window_cleanup();

#endif //BEETROOT_WINDOW_H
