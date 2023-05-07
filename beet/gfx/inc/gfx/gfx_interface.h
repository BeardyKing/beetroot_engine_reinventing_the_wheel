#ifndef BEETROOT_GFX_INTERFACE_H
#define BEETROOT_GFX_INTERFACE_H

#include <vulkan/vulkan_core.h>

//===api=====================
VkInstance* gfx_instance();
VkSurfaceKHR* gfx_surface();

//===init & shutdown=========
void gfx_create();
void gfx_cleanup();

#endif //BEETROOT_GFX_INTERFACE_H
