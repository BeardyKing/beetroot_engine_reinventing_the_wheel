//===defines=================
#include <gfx/gfx_interface.h>
#include <gfx/vulkan_platform_defines.h>

#include <shared/log.h>
#include <shared/assert.h>

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

//===internal structs========
struct GfxDevice {
    VkSurfaceKHR vkSurface{};
    VkInstance vkInstance{};
    VkPhysicalDevice vkPhysicalDevice{};

    VkDebugUtilsMessengerEXT vkDebugUtilsMessengerExt = VK_NULL_HANDLE;
};

GfxDevice *g_gfxDevice;

struct VulkanProperties {
    VkExtensionProperties *supportedExtensions{};
    uint32_t extensionsCount{};

    VkLayerProperties *supportedValidationLayers{};
    uint32_t validationLayersCount{};

    VkPhysicalDeviceProperties selectedPhysicalDevice{};
};

VulkanProperties *g_vulkanProperties;

struct UserArguments {
    uint32_t selectedPhysicalDeviceIndex{};
};

UserArguments *g_userArguments;

//===internal mappings=======
static const char *BEET_VK_PHYSICAL_DEVICE_TYPE_MAPPING[] = {
        "VK_PHYSICAL_DEVICE_TYPE_OTHER",
        "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU",
        "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU",
        "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU",
        "VK_PHYSICAL_DEVICE_TYPE_CPU",
};

//===internal functions======
void store_supported_extensions() {
    vkEnumerateInstanceExtensionProperties(nullptr, &g_vulkanProperties->extensionsCount, nullptr);
    g_vulkanProperties->supportedExtensions = new VkExtensionProperties[g_vulkanProperties->extensionsCount];
    if (g_vulkanProperties->extensionsCount > 0) {
        vkEnumerateInstanceExtensionProperties(nullptr,
                                               &g_vulkanProperties->extensionsCount,
                                               g_vulkanProperties->supportedExtensions);
    }

    for (uint32_t i = 0; i < g_vulkanProperties->extensionsCount; ++i) {
        log_verbose("Extension: %s \n", g_vulkanProperties->supportedExtensions[i].extensionName);
    }
}

void store_supported_validation_layers() {
    vkEnumerateInstanceLayerProperties(&g_vulkanProperties->validationLayersCount, nullptr);
    g_vulkanProperties->supportedValidationLayers = new VkLayerProperties[g_vulkanProperties->validationLayersCount];
    if (g_vulkanProperties->validationLayersCount > 0) {
        vkEnumerateInstanceLayerProperties(&g_vulkanProperties->validationLayersCount,
                                           g_vulkanProperties->supportedValidationLayers);
    }

    for (uint32_t i = 0; i < g_vulkanProperties->validationLayersCount; ++i) {
        log_verbose("Layer: %s - Desc: %s \n", g_vulkanProperties->supportedValidationLayers[i].layerName,
                    g_vulkanProperties->supportedValidationLayers[i].description);
    }
}

void validate_extensions() {
    for (uint8_t i = 0; i < BEET_VK_EXTENSION_COUNT; i++) {
        bool result = gfx_find_supported_extension(vulkanExtensions[i]);
        ASSERT_MSG(result, "Err: failed find support for extension [%s]", vulkanExtensions[i]);
    }
}


void validate_validation_layers() {
    for (uint8_t i = 0; i < BEET_VK_VALIDATION_COUNT; i++) {
        bool result = gfx_find_supported_validation(vulkanValidations[i]);
        ASSERT_MSG(result, "Err: failed find support for validation layer [%s]", vulkanValidations[i]);
    }
}

static VkBool32 VKAPI_PTR validation_message_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageWarningLevel,
        VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
        const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
        void */*userData*/) {

    switch (messageWarningLevel) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
            log_error("\ncode: \t\t%s \nmessage: \t%s\n", callbackData->pMessageIdName, callbackData->pMessage);
            break;
        }
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
            log_warning("\ncode: \t\t%s \nmessage: \t%s\n", callbackData->pMessageIdName, callbackData->pMessage);
            break;
        }
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
            log_info("\ncode: \t\t%s \nmessage: \t%s\n", callbackData->pMessageIdName, callbackData->pMessage);
            break;
        }
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
            log_verbose("\ncode: \t\t%s \nmessage: \t%s\n", callbackData->pMessageIdName, callbackData->pMessage);
            break;
        }
        default: {
            ASSERT(callbackData && callbackData->pMessageIdName && callbackData->pMessage);
        }
    }
    return VK_FALSE;
}

//===api=====================
VkInstance *gfx_instance() {
    return &g_gfxDevice->vkInstance;
}

VkSurfaceKHR *gfx_surface() {
    return &g_gfxDevice->vkSurface;
}

bool gfx_find_supported_extension(const char *extensionName) {
    for (uint32_t i = 0; i < g_vulkanProperties->extensionsCount; ++i) {
        if (strcmp(g_vulkanProperties->supportedExtensions[i].extensionName, extensionName) == 0) {
            return true;
        }
    }
    return false;
}

bool gfx_find_supported_validation(const char *layerName) {
    for (uint32_t i = 0; i < g_vulkanProperties->validationLayersCount; ++i) {
        if (strcmp(g_vulkanProperties->supportedValidationLayers[i].layerName, layerName) == 0) {
            return true;
        }
    }
    return false;
}

//===init & shutdown=========
void gfx_create() {
    g_gfxDevice = new GfxDevice;
    g_vulkanProperties = new VulkanProperties;
    g_userArguments = new UserArguments;
}

void gfx_cleanup() {
    {
        delete g_gfxDevice;
        g_gfxDevice = nullptr;
    }
    {
        delete[] g_vulkanProperties->supportedExtensions;
        g_vulkanProperties->supportedExtensions = nullptr;

        delete[] g_vulkanProperties->supportedValidationLayers;
        g_vulkanProperties->supportedValidationLayers = nullptr;

        delete g_vulkanProperties;
        g_vulkanProperties = nullptr;
    }
    {
        delete g_userArguments;
    }
}

void gfx_create_instance() {
    store_supported_extensions();
    validate_extensions();

    store_supported_validation_layers();
    validate_validation_layers();

    VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.pApplicationName = "VK_BEETROOT_ENGINE";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.pEngineName = "VK_BEETROOT_ENGINE";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = BEET_MAX_VK_API_VERSION;

    VkInstanceCreateInfo instInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    instInfo.pApplicationInfo = &appInfo;
    instInfo.enabledExtensionCount = BEET_VK_EXTENSION_COUNT;
    instInfo.ppEnabledExtensionNames = vulkanExtensions;
    instInfo.enabledLayerCount = BEET_VK_VALIDATION_COUNT;
    instInfo.ppEnabledLayerNames = vulkanValidations;

    auto result = vkCreateInstance(&instInfo, nullptr, &g_gfxDevice->vkInstance);
    ASSERT_MSG(result == VK_SUCCESS, "Err: failed to create vulkan instance");
}

void gfx_cleanup_instance() {
    ASSERT_MSG(g_gfxDevice->vkInstance != VK_NULL_HANDLE, "Err: VkInstance has already been destroyed");
    vkDestroyInstance(g_gfxDevice->vkInstance, nullptr);
    g_gfxDevice->vkInstance = VK_NULL_HANDLE;
}

void gfx_create_debug_callbacks() {
    VkInstance &vkInstance = g_gfxDevice->vkInstance;
    vkCreateDebugUtilsMessengerEXT_Func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
            vkInstance, BEET_VK_CREATE_DEBUG_UTIL_EXT);

    ASSERT_MSG(vkCreateDebugUtilsMessengerEXT_Func, "Err: failed to setup debug callback %s",
               BEET_VK_CREATE_DEBUG_UTIL_EXT);

    vkDestroyDebugUtilsMessengerEXT_Func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
            vkInstance, BEET_VK_DESTROY_DEBUG_UTIL_EXT);

    ASSERT_MSG(vkDestroyDebugUtilsMessengerEXT_Func, "Err: failed to setup debug callback %s",
               BEET_VK_DESTROY_DEBUG_UTIL_EXT);

    vkSetDebugUtilsObjectNameEXT_Func = (PFN_vkSetDebugUtilsObjectNameEXT) vkGetInstanceProcAddr(
            vkInstance, BEET_VK_OBJECT_NAME_DEBUG_UTIL_EXT);

    ASSERT_MSG(vkSetDebugUtilsObjectNameEXT_Func, "Err: failed to setup debug callback %s",
               BEET_VK_OBJECT_NAME_DEBUG_UTIL_EXT);

    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    messengerCreateInfo.messageSeverity = BEET_VK_DEBUG_UTILS_MESSAGE_SEVERITY;
    messengerCreateInfo.messageType = BEET_VK_DEBUG_UTILS_MESSAGE_TYPE;
    messengerCreateInfo.pfnUserCallback = validation_message_callback;
    vkCreateDebugUtilsMessengerEXT_Func(vkInstance,
                                        &messengerCreateInfo,
                                        nullptr,
                                        &g_gfxDevice->vkDebugUtilsMessengerExt);
}

void gfx_cleanup_debug_callbacks() {
    ASSERT_MSG(g_gfxDevice->vkDebugUtilsMessengerExt != VK_NULL_HANDLE,
               "Err: debug utils messenger has already been destroyed");
    vkDestroyDebugUtilsMessengerEXT_Func(g_gfxDevice->vkInstance, g_gfxDevice->vkDebugUtilsMessengerExt, nullptr);
    g_gfxDevice->vkDebugUtilsMessengerExt = VK_NULL_HANDLE;
}

void gfx_cleanup_surface() {
    ASSERT_MSG(g_gfxDevice->vkSurface != VK_NULL_HANDLE, "Err: VkSurface has already been destroyed");
    vkDestroySurfaceKHR(g_gfxDevice->vkInstance, g_gfxDevice->vkSurface, nullptr);
    g_gfxDevice->vkSurface = VK_NULL_HANDLE;
}

void gfx_create_physical_device() {
    VkPhysicalDevice *physicalDevices = nullptr;
    uint32_t deviceCount = 0;
    uint32_t selectedDevice = 0;

    vkEnumeratePhysicalDevices(g_gfxDevice->vkInstance, &deviceCount, nullptr);
    ASSERT_MSG(deviceCount != 0, "Err: did not find any vulkan compatible physical devices");

    physicalDevices = new VkPhysicalDevice[deviceCount];
    vkEnumeratePhysicalDevices(g_gfxDevice->vkInstance, &deviceCount, physicalDevices);

    //TODO:GFX: Add fallback support for `best` GPU based on intended workload, if no argument is provided we fallback to device [0]
    selectedDevice = g_userArguments->selectedPhysicalDeviceIndex;
#if BEET_DEBUG
    if (BEET_DEBUG_VK_FORCE_GPU_SELECTION > -1) {
        selectedDevice = BEET_DEBUG_VK_FORCE_GPU_SELECTION;
        log_warning("Using debug ONLY feature `BEET_VK_FORCE_GPU_SELECTION` selecting device [%i]\n",
                    selectedDevice)
    }
#endif

    ASSERT_MSG(selectedDevice < deviceCount, "Err: selecting physical vulkan device that is out of range");
    g_gfxDevice->vkPhysicalDevice = physicalDevices[selectedDevice];

    vkGetPhysicalDeviceProperties(g_gfxDevice->vkPhysicalDevice, &g_vulkanProperties->selectedPhysicalDevice);

    const uint32_t apiVersionMajor = VK_API_VERSION_MAJOR(g_vulkanProperties->selectedPhysicalDevice.apiVersion);
    const uint32_t apiVersionMinor = VK_API_VERSION_MINOR(g_vulkanProperties->selectedPhysicalDevice.apiVersion);
    const uint32_t apiVersionPatch = VK_API_VERSION_PATCH(g_vulkanProperties->selectedPhysicalDevice.apiVersion);

    const uint32_t driverVersionMajor = VK_API_VERSION_MAJOR(g_vulkanProperties->selectedPhysicalDevice.driverVersion);
    const uint32_t driverVersionMinor = VK_API_VERSION_MINOR(g_vulkanProperties->selectedPhysicalDevice.driverVersion);
    const uint32_t driverVersionPatch = VK_API_VERSION_PATCH(g_vulkanProperties->selectedPhysicalDevice.driverVersion);

    const char *deviceType = BEET_VK_PHYSICAL_DEVICE_TYPE_MAPPING[g_vulkanProperties->selectedPhysicalDevice.deviceType];

    log_info("\nName:\t\t%s \nType:\t\t%s \nVersion:\t%u.%u.%u \nDriver: \t%u.%u.%u\n",
             g_vulkanProperties->selectedPhysicalDevice.deviceName,
             deviceType,
             apiVersionMajor,
             apiVersionMinor,
             apiVersionPatch,
             driverVersionMajor,
             driverVersionMinor,
             driverVersionPatch
    );

    delete[] physicalDevices;
    physicalDevices = nullptr;
}

void gfx_cleanup_physical_device() {
    g_gfxDevice->vkPhysicalDevice = VK_NULL_HANDLE;
}

void gfx_select_physical_device(uint32_t deviceIndex) {
    g_userArguments->selectedPhysicalDeviceIndex = deviceIndex;
}