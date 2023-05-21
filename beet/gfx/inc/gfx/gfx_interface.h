#ifndef BEETROOT_GFX_INTERFACE_H
#define BEETROOT_GFX_INTERFACE_H

#include <vulkan/vulkan_core.h>

//===api=====================
VkInstance* gfx_instance();
VkSurfaceKHR* gfx_surface();

bool gfx_find_supported_extension(const char *extensionName);
bool gfx_find_supported_validation(const char *layerName);

void gfx_select_physical_device(uint32_t deviceIndex);

//===init & shutdown=========
void gfx_create();
void gfx_cleanup();

void gfx_create_instance();
void gfx_cleanup_instance();

//Note: create surface handled by windowing platform.
void gfx_cleanup_surface();

void gfx_create_debug_callbacks();
void gfx_cleanup_debug_callbacks();

void gfx_create_physical_device();
void gfx_cleanup_physical_device();

#endif //BEETROOT_GFX_INTERFACE_H
