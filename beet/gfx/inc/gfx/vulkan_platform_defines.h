#ifndef BEETROOT_VULKAN_PLATFORM_DEFINES_H
#define BEETROOT_VULKAN_PLATFORM_DEFINES_H

//===surface=================
#if defined (_WIN32)
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <vulkan/vulkan_win32.h>

#define BEET_VK_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined(__linux__)
#endif

//===extensions==============
const int BEET_VK_EXTENSION_COUNT = 2;
const char *vulkanExtensions[BEET_VK_EXTENSION_COUNT]{
        VK_KHR_SURFACE_EXTENSION_NAME,
        BEET_VK_SURFACE_EXTENSION_NAME,
};

//===validation==============
#define BEET_VK_LAYER_VALIDATION "VK_LAYER_KHRONOS_validation"

const int BEET_VK_VALIDATION_COUNT = 1;
const char *vulkanValidations[BEET_VK_EXTENSION_COUNT]{
        BEET_VK_LAYER_VALIDATION,
};

#endif //BEETROOT_VULKAN_PLATFORM_DEFINES_H
