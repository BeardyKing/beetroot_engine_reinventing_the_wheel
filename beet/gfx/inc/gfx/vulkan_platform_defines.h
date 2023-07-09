#ifndef BEETROOT_VULKAN_PLATFORM_DEFINES_H
#define BEETROOT_VULKAN_PLATFORM_DEFINES_H

//#include <gfx_headers.h>

//===runtime=================
static const uint32_t BEET_VK_COMMAND_BUFFER_COUNT = 2;

//===surface=================
#if defined (_WIN32)
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <vulkan/vulkan_win32.h>

#define BEET_VK_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined(__linux__)
#endif

//==debug====================
#if BEET_DEBUG
static const int BEET_DEBUG_VK_FORCE_GPU_SELECTION = 0; // ignore [-1] force select [0 .. UINT32_MAX]
#endif

//===extensions==============
static const int BEET_VK_EXTENSION_COUNT = 3;
static const char *beetVulkanExtensions[BEET_VK_EXTENSION_COUNT]{
        VK_KHR_SURFACE_EXTENSION_NAME,
        BEET_VK_SURFACE_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
};

//===validation==============
#define BEET_VK_LAYER_VALIDATION "VK_LAYER_KHRONOS_validation"

static const int BEET_VK_VALIDATION_COUNT = 1;
static const char *beetVulkanValidations[BEET_VK_EXTENSION_COUNT]{
        BEET_VK_LAYER_VALIDATION,
};

//===debug msg callbacks=====
static PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT_Func;
static PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT_Func;
static PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT_Func;

#define BEET_VK_CREATE_DEBUG_UTIL_EXT "vkCreateDebugUtilsMessengerEXT"
#define BEET_VK_DESTROY_DEBUG_UTIL_EXT "vkDestroyDebugUtilsMessengerEXT"
#define BEET_VK_OBJECT_NAME_DEBUG_UTIL_EXT "vkSetDebugUtilsObjectNameEXT"

static const VkDebugUtilsMessageSeverityFlagsEXT BEET_VK_DEBUG_UTILS_MESSAGE_SEVERITY =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

static const VkDebugUtilsMessageTypeFlagsEXT BEET_VK_DEBUG_UTILS_MESSAGE_TYPE =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

//===api version=============
#if defined(VK_VERSION_1_3)
#define BEET_MAX_VK_API_VERSION VK_API_VERSION_1_3
#elif defined(VK_VERSION_1_2)
#define BEET_MAX_VK_API_VERSION VK_API_VERSION_1_2
#elif defined(VK_VERSION_1_1)
#define BEET_MAX_VK_API_VERSION VK_API_VERSION_1_1
#elif defined(VK_VERSION_1_0)
#define BEET_MAX_VK_API_VERSION VK_API_VERSION_1_0
#else
SANITY_CHECK()
#endif

#endif //BEETROOT_VULKAN_PLATFORM_DEFINES_H
