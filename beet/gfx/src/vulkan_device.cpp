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
    VkSurfaceKHR vkSurface;
    VkInstance vkInstance;
};

GfxDevice *g_gfxDevice;

struct VulkanProperties {
    VkExtensionProperties *supportedExtensions{};
    uint32_t extensionsCount{};

    VkLayerProperties *supportedValidationLayers{};
    uint32_t validationLayersCount{};
};

VulkanProperties *g_vulkanProperties;


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

bool find_supported_extension(const char *extensionName) {
    for (uint32_t i = 0; i < g_vulkanProperties->extensionsCount; ++i) {
        if (strcmp(g_vulkanProperties->supportedExtensions[i].extensionName, extensionName) == 0) {
            return true;
        }
    }
    return false;
}

void validate_extensions() {
    for (uint8_t i = 0; i < BEET_VK_EXTENSION_COUNT; i++) {
        bool result = find_supported_extension(vulkanExtensions[i]);
        ASSERT_MSG(result, "Err: failed find support for extension [%s]", vulkanExtensions[i]);
    }
}

bool find_supported_validation(const char *layerName) {
    for (uint32_t i = 0; i < g_vulkanProperties->validationLayersCount; ++i) {
        if (strcmp(g_vulkanProperties->supportedValidationLayers[i].layerName, layerName) == 0) {
            return true;
        }
    }
    return false;
}

void validate_validation_layers() {
    for (uint8_t i = 0; i < BEET_VK_VALIDATION_COUNT; i++) {
        bool result = find_supported_validation(vulkanValidations[i]);
        ASSERT_MSG(result, "Err: failed find support for validation layer [%s]", vulkanValidations[i]);
    }
}

void setup_vulkan_instance() {
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

//===api=====================
VkInstance *gfx_instance() {
    return &g_gfxDevice->vkInstance;
}

VkSurfaceKHR *gfx_surface() {
    return &g_gfxDevice->vkSurface;
}

//===init & shutdown=========
void gfx_create() {
    g_gfxDevice = new GfxDevice;
    g_vulkanProperties = new VulkanProperties;
    setup_vulkan_instance();
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
}