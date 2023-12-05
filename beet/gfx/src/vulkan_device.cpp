//===defines=================
#include <gfx/gfx_interface.h>
#include <gfx/gfx_types.h>
#include <gfx/gfx_lit.h>
#include <gfx/gfx_font.h>
#include <gfx/vulkan_platform_defines.h>
#include <gfx/gfx_samplers.h>

#include <shared/log.h>
#include <shared/assert.h>
#include <shared/bit_utils.h>

#include <unordered_map>

#define VMA_IMPLEMENTATION

#include <vk_mem_alloc.h>

//===internal structs========

GfxDevice *g_gfxDevice;

struct VulkanProperties {
    VkExtensionProperties *supportedExtensions{};
    uint32_t extensionsCount{};

    VkLayerProperties *supportedValidationLayers{};
    uint32_t validationLayersCount{};

    VkPhysicalDeviceProperties selectedPhysicalDevice{};
    VkPhysicalDeviceFeatures selectedPhysicalDeviceFeatures{};
};

VulkanProperties *g_vulkanProperties;

struct UserArguments {
    uint32_t selectedPhysicalDeviceIndex{};
    bool vsync{true};
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
        vkEnumerateInstanceExtensionProperties(nullptr, &g_vulkanProperties->extensionsCount, g_vulkanProperties->supportedExtensions);
    }

    for (uint32_t i = 0; i < g_vulkanProperties->extensionsCount; ++i) {
        log_verbose("Extension: %s \n", g_vulkanProperties->supportedExtensions[i].extensionName);
    }
}

void store_supported_validation_layers() {
    vkEnumerateInstanceLayerProperties(&g_vulkanProperties->validationLayersCount, nullptr);
    g_vulkanProperties->supportedValidationLayers = new VkLayerProperties[g_vulkanProperties->validationLayersCount];
    if (g_vulkanProperties->validationLayersCount > 0) {
        vkEnumerateInstanceLayerProperties(&g_vulkanProperties->validationLayersCount, g_vulkanProperties->supportedValidationLayers);
    }

    for (uint32_t i = 0; i < g_vulkanProperties->validationLayersCount; ++i) {
        log_verbose("Layer: %s - Desc: %s \n",
                    g_vulkanProperties->supportedValidationLayers[i].layerName,
                    g_vulkanProperties->supportedValidationLayers[i].description);
    }
}

void validate_extensions() {
    for (uint8_t i = 0; i < BEET_VK_EXTENSION_COUNT; i++) {
        bool result = gfx_find_supported_extension(beetVulkanExtensions[i]);
        ASSERT_MSG(result, "Err: failed find support for extension [%s]", beetVulkanExtensions[i]);
    }
}

void validate_validation_layers() {
    for (uint8_t i = 0; i < BEET_VK_VALIDATION_COUNT; i++) {
        bool result = gfx_find_supported_validation(beetVulkanValidations[i]);
        ASSERT_MSG(result, "Err: failed find support for validation layer [%s]", beetVulkanValidations[i]);
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

void begin_command_recording(const VkCommandBuffer &cmdBuffer) {
    VkCommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = nullptr;
    cmdBeginInfo.pInheritanceInfo = nullptr;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    auto result = vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo);
    ASSERT_MSG(result == VK_SUCCESS, "Err: Vulkan failed to begin command buffer recording");
}

void end_command_recording(const VkCommandBuffer &cmdBuffer) {
    vkEndCommandBuffer(cmdBuffer);
}


VkResult acquire_next_swapchain_image() {
    return vkAcquireNextImageKHR(
            g_gfxDevice->vkDevice,
            g_gfxDevice->vkSwapchain,
            UINT64_MAX,
            g_gfxDevice->vkSemaphoreImageAvailable,
            VK_NULL_HANDLE,
            &g_gfxDevice->swapchainImageIndex
    );
}

bool query_has_valid_extent_size() {
    VkSurfaceCapabilitiesKHR surfaceCapabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            g_gfxDevice->vkPhysicalDevice,
            g_gfxDevice->vkSurface,
            &surfaceCapabilities
    );
    return surfaceCapabilities.currentExtent.width != 0 || surfaceCapabilities.currentExtent.height != 0;
}

void destroy_swapchain() {
    {
        vkDestroySemaphore(g_gfxDevice->vkDevice, g_gfxDevice->vkSemaphoreImageAvailable, nullptr);
        g_gfxDevice->vkSemaphoreImageAvailable = VK_NULL_HANDLE;

        vkDestroySemaphore(g_gfxDevice->vkDevice, g_gfxDevice->vkSemaphoreRenderFinished, nullptr);
        g_gfxDevice->vkSemaphoreRenderFinished = VK_NULL_HANDLE;
    }
    {
        gfx_destroy_lit();
        gfx_destroy_font();
    }
    {
        vkDestroyImageView(g_gfxDevice->vkDevice, g_gfxDevice->depthImageView, nullptr);
        g_gfxDevice->depthImageView = VK_NULL_HANDLE;

        vmaDestroyImage(g_gfxDevice->vmaAllocator, g_gfxDevice->depthImage, g_gfxDevice->depthAllocation);
        g_gfxDevice->depthImageView = VK_NULL_HANDLE;
    }
    {
        for (uint32_t i = 0; i < g_gfxDevice->swapchainImageViewCount; ++i) {
            vkDestroyImageView(g_gfxDevice->vkDevice, g_gfxDevice->vkSwapchainImageViews[i], nullptr);
        }
        g_gfxDevice->swapchainImageViewCount = 0;
        delete[] g_gfxDevice->vkSwapchainImageViews;
        g_gfxDevice->vkSwapchainImageViews = nullptr;
    }
}

void gfx_command_submit(const VkCommandBuffer &cmdBuffer) {
    const uint32_t submitWaitSemaphoresCount = 1;
    VkSemaphore submitWaitSemaphores[submitWaitSemaphoresCount] = {g_gfxDevice->vkSemaphoreImageAvailable};

    const uint32_t submitSignalSemaphoresCount = 1;
    VkSemaphore submitSignalSemaphores[submitSignalSemaphoresCount] = {g_gfxDevice->vkSemaphoreRenderFinished};

    VkPipelineStageFlags submitWaitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};

    submitInfo.waitSemaphoreCount = submitWaitSemaphoresCount;
    submitInfo.pWaitSemaphores = submitWaitSemaphores;
    submitInfo.pWaitDstStageMask = submitWaitStages;
    submitInfo.pCommandBuffers = &cmdBuffer;
    submitInfo.commandBufferCount = 1;
    submitInfo.signalSemaphoreCount = submitSignalSemaphoresCount;
    submitInfo.pSignalSemaphores = submitSignalSemaphores;

    vkQueueSubmit(
            g_gfxDevice->vkGraphicsQueue,
            1,
            &submitInfo,
            g_gfxDevice->vkGraphicsFences[g_gfxDevice->nextCommandBufferIndex]
    );
}

void preset_queue() {
    const uint32_t presentWaitSemaphoresCount = 1;
    VkSemaphore presentWaitSemaphores[presentWaitSemaphoresCount] = {g_gfxDevice->vkSemaphoreRenderFinished};

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = presentWaitSemaphoresCount;
    presentInfo.pWaitSemaphores = presentWaitSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &g_gfxDevice->vkSwapchain;
    presentInfo.pImageIndices = &g_gfxDevice->swapchainImageIndex;
    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(g_gfxDevice->vkPresentQueue, &presentInfo);
}

VkFormat find_depth_format(const VkImageTiling &desiredTilingFormat) {
    VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    const uint32_t formatCandidateCount = 3;
    VkFormat candidates[formatCandidateCount]{
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT
    };

    for (VkFormat format: candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(g_gfxDevice->vkPhysicalDevice, format, &props);

        if ((desiredTilingFormat == VK_IMAGE_TILING_LINEAR) &&
            ((props.linearTilingFeatures & features) == features)) {
            return format;
        }
        if ((desiredTilingFormat == VK_IMAGE_TILING_OPTIMAL) &&
            ((props.optimalTilingFeatures & features) == features)) {
            return format;
        }
    }
    ASSERT_MSG(false, "Err: could not find supported depth format");
    return VK_FORMAT_UNDEFINED;
}

VkPresentModeKHR select_present_mode() {
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(g_gfxDevice->vkPhysicalDevice, g_gfxDevice->vkSurface, &presentModeCount, nullptr);

    VkPresentModeKHR *presentModes = new VkPresentModeKHR[presentModeCount];
    vkGetPhysicalDeviceSurfacePresentModesKHR(g_gfxDevice->vkPhysicalDevice, g_gfxDevice->vkSurface, &presentModeCount, presentModes);

    VkPresentModeKHR selectedPresentMode{VK_PRESENT_MODE_FIFO_KHR};
    VkPresentModeKHR preferredMode = g_userArguments->vsync ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
    for (uint32_t i = 0; i < presentModeCount; ++i) {
        if (presentModes[i] == preferredMode) {
            selectedPresentMode = presentModes[i];
            break;
        }
    }
    delete[] presentModes;
    return selectedPresentMode;
}

VkSurfaceFormatKHR select_surface_format() {
    uint32_t surfaceFormatsCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(g_gfxDevice->vkPhysicalDevice, g_gfxDevice->vkSurface, &surfaceFormatsCount, nullptr);

    VkSurfaceFormatKHR *surfaceFormats = new VkSurfaceFormatKHR[surfaceFormatsCount];
    vkGetPhysicalDeviceSurfaceFormatsKHR(g_gfxDevice->vkPhysicalDevice, g_gfxDevice->vkSurface, &surfaceFormatsCount, surfaceFormats);

    // Create swap chain
    //select best surface format
    VkSurfaceFormatKHR selectedSurfaceFormat{};
    if ((surfaceFormatsCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED)) {
        VkSurfaceFormatKHR format = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        selectedSurfaceFormat = format;
    } else {
        for (uint32_t i = 0; i < surfaceFormatsCount; ++i) {
            if ((surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_UNORM) &&
                (surfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)) {
                selectedSurfaceFormat = surfaceFormats[i];
                break;
            }
        }
    }
    delete[]surfaceFormats;
    return selectedSurfaceFormat;
}

void create_swapchain(const VkSurfaceFormatKHR &surfaceFormat, const VkPresentModeKHR &presentMode) {
    VkSurfaceCapabilitiesKHR surfaceCapabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            g_gfxDevice->vkPhysicalDevice,
            g_gfxDevice->vkSurface,
            &surfaceCapabilities
    );

    if (surfaceCapabilities.currentExtent.width == UINT32_MAX ||
        surfaceCapabilities.currentExtent.height == UINT32_MAX) {
        g_gfxDevice->vkExtent.width = 1280;
        g_gfxDevice->vkExtent.height = 720;
    } else {
        g_gfxDevice->vkExtent = surfaceCapabilities.currentExtent;
    }


    uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
    if ((surfaceCapabilities.maxImageCount > 0) &&
        (imageCount > surfaceCapabilities.maxImageCount)) {
        imageCount = surfaceCapabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapChainInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    swapChainInfo.surface = g_gfxDevice->vkSurface;
    swapChainInfo.minImageCount = imageCount;
    swapChainInfo.imageFormat = surfaceFormat.format;
    swapChainInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapChainInfo.imageExtent = g_gfxDevice->vkExtent;
    swapChainInfo.imageArrayLayers = 1;
    swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapChainInfo.preTransform = surfaceCapabilities.currentTransform;
    swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainInfo.presentMode = presentMode;
    swapChainInfo.clipped = VK_TRUE;
    swapChainInfo.oldSwapchain = g_gfxDevice->vkSwapchain;

    uint32_t queueFamilyIndices[] = {g_gfxDevice->graphicsQueueIndex, g_gfxDevice->presentQueueIndex};
    if (g_gfxDevice->graphicsQueueIndex != g_gfxDevice->presentQueueIndex) {
        swapChainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapChainInfo.queueFamilyIndexCount = 2;
        swapChainInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    vkCreateSwapchainKHR(g_gfxDevice->vkDevice, &swapChainInfo, nullptr, &g_gfxDevice->vkSwapchain);
}

void create_swapchain_image_views(const VkFormat selectedSurfaceFormat) {
    uint32_t swapchainImagesCount = 0;
    vkGetSwapchainImagesKHR(g_gfxDevice->vkDevice, g_gfxDevice->vkSwapchain, &swapchainImagesCount, nullptr);

    VkImage *swapchainImages = new VkImage[swapchainImagesCount];
    vkGetSwapchainImagesKHR(g_gfxDevice->vkDevice, g_gfxDevice->vkSwapchain, &swapchainImagesCount, swapchainImages);

    g_gfxDevice->swapchainImageViewCount = swapchainImagesCount;
    VkImageViewCreateInfo swapchainImageViewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    g_gfxDevice->vkSwapchainImageViews = new VkImageView[g_gfxDevice->swapchainImageViewCount];

    for (uint32_t i = 0; i < swapchainImagesCount; ++i) {
        swapchainImageViewInfo.image = swapchainImages[i];
        swapchainImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        swapchainImageViewInfo.format = selectedSurfaceFormat;
        swapchainImageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        swapchainImageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        swapchainImageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        swapchainImageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        swapchainImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        swapchainImageViewInfo.subresourceRange.baseMipLevel = 0;
        swapchainImageViewInfo.subresourceRange.levelCount = 1;
        swapchainImageViewInfo.subresourceRange.baseArrayLayer = 0;
        swapchainImageViewInfo.subresourceRange.layerCount = 1;
        vkCreateImageView(g_gfxDevice->vkDevice, &swapchainImageViewInfo, nullptr,
                          &g_gfxDevice->vkSwapchainImageViews[i]);
    }
    delete[] swapchainImages;
}

void create_depth_buffer(const VkFormat &selectedDepthFormat) {
    VkImageCreateInfo depthImageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
    depthImageInfo.extent.width = g_gfxDevice->vkExtent.width;
    depthImageInfo.extent.height = g_gfxDevice->vkExtent.height;
    depthImageInfo.extent.depth = 1;
    depthImageInfo.mipLevels = 1;
    depthImageInfo.arrayLayers = 1;
    depthImageInfo.format = selectedDepthFormat;
    depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    depthImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depthImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    depthImageInfo.flags = 0;

    VmaAllocationCreateInfo depthImageAllocCreateInfo = {};
    depthImageAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

    vmaCreateImage(
            g_gfxDevice->vmaAllocator,
            &depthImageInfo,
            &depthImageAllocCreateInfo,
            &g_gfxDevice->depthImage,
            &g_gfxDevice->depthAllocation,
            nullptr
    );

    VkImageViewCreateInfo depthImageViewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    depthImageViewInfo.image = g_gfxDevice->depthImage;
    depthImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthImageViewInfo.format = selectedDepthFormat;
    depthImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthImageViewInfo.subresourceRange.baseMipLevel = 0;
    depthImageViewInfo.subresourceRange.levelCount = 1;
    depthImageViewInfo.subresourceRange.baseArrayLayer = 0;
    depthImageViewInfo.subresourceRange.layerCount = 1;

    vkCreateImageView(g_gfxDevice->vkDevice, &depthImageViewInfo, nullptr, &g_gfxDevice->depthImageView);
}


void create_semaphores() {
    VkSemaphoreCreateInfo semaphoreInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    vkCreateSemaphore(g_gfxDevice->vkDevice, &semaphoreInfo, nullptr, &g_gfxDevice->vkSemaphoreImageAvailable);
    vkCreateSemaphore(g_gfxDevice->vkDevice, &semaphoreInfo, nullptr, &g_gfxDevice->vkSemaphoreRenderFinished);
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
    instInfo.ppEnabledExtensionNames = beetVulkanExtensions;
    instInfo.enabledLayerCount = BEET_VK_VALIDATION_COUNT;
    instInfo.ppEnabledLayerNames = beetVulkanValidations;

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
    vkGetPhysicalDeviceFeatures(g_gfxDevice->vkPhysicalDevice, &g_vulkanProperties->selectedPhysicalDeviceFeatures);

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
}

void gfx_cleanup_physical_device() {
    g_gfxDevice->vkPhysicalDevice = VK_NULL_HANDLE;
}

void gfx_select_physical_device(uint32_t deviceIndex) {
    g_userArguments->selectedPhysicalDeviceIndex = deviceIndex;
}


void gfx_create_queues() {
    uint32_t devicePropertyCount = 0;
    vkEnumerateDeviceExtensionProperties(g_gfxDevice->vkPhysicalDevice, nullptr, &devicePropertyCount, nullptr);
    VkExtensionProperties *selectedPhysicalDeviceExtensions = new VkExtensionProperties[devicePropertyCount];

    if (devicePropertyCount > 0) {
        vkEnumerateDeviceExtensionProperties(g_gfxDevice->vkPhysicalDevice, nullptr, &devicePropertyCount, selectedPhysicalDeviceExtensions);
    }

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(g_gfxDevice->vkPhysicalDevice, &queueFamilyCount, nullptr);
    ASSERT(queueFamilyCount != 0);

    VkQueueFamilyProperties *queueFamilies = new VkQueueFamilyProperties[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(g_gfxDevice->vkPhysicalDevice, &queueFamilyCount, queueFamilies);

    struct QueueInfo {
        uint32_t targetFlags{UINT32_MAX};
        uint32_t queueIndex{UINT32_MAX};
        uint32_t currentFlags{UINT32_MAX};
        bool supportsPresent{false};
    };

    QueueInfo graphicsQueueInfo;
    graphicsQueueInfo.targetFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;

    QueueInfo transferQueueInfo;
    transferQueueInfo.targetFlags = VK_QUEUE_TRANSFER_BIT;

    QueueInfo presentQueueInfo;
    presentQueueInfo.targetFlags = 0;

    for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; ++queueFamilyIndex) {
        VkBool32 supportsPresent = 0;
        VkResult presentResult = vkGetPhysicalDeviceSurfaceSupportKHR(
                g_gfxDevice->vkPhysicalDevice,
                queueFamilyIndex,
                g_gfxDevice->vkSurface,
                &supportsPresent
        );

        const uint32_t currentQueueFlags = queueFamilies[queueFamilyIndex].queueFlags;

        if ((presentResult >= 0) && (supportsPresent == VK_TRUE)) {
            if (count_set_bits(currentQueueFlags) < count_set_bits(presentQueueInfo.currentFlags)
                || currentQueueFlags != graphicsQueueInfo.queueIndex) {
                presentQueueInfo.currentFlags = currentQueueFlags;
                presentQueueInfo.queueIndex = queueFamilyIndex;
                presentQueueInfo.supportsPresent = supportsPresent;
            }
        }

        if (currentQueueFlags & graphicsQueueInfo.targetFlags) {
            if (count_set_bits(currentQueueFlags) < count_set_bits(graphicsQueueInfo.currentFlags)) {
                graphicsQueueInfo.currentFlags = currentQueueFlags;
                graphicsQueueInfo.queueIndex = queueFamilyIndex;
                graphicsQueueInfo.supportsPresent = supportsPresent;
                continue;
            }
        }

        if (currentQueueFlags & transferQueueInfo.targetFlags) {
            if (count_set_bits(currentQueueFlags) < count_set_bits(transferQueueInfo.currentFlags)) {
                transferQueueInfo.currentFlags = currentQueueFlags;
                transferQueueInfo.queueIndex = queueFamilyIndex;
                transferQueueInfo.supportsPresent = supportsPresent;
                continue;
            }
        }
    }

    log_verbose("present queue index:  %u \n", presentQueueInfo.queueIndex);
    log_verbose("graphics queue index: %u \n", graphicsQueueInfo.queueIndex);
    log_verbose("transfer queue index: %u \n", transferQueueInfo.queueIndex);

    ASSERT_MSG(presentQueueInfo.queueIndex != UINT32_MAX, "Err: did not find present queue");
    ASSERT_MSG(graphicsQueueInfo.queueIndex != UINT32_MAX, "Err: did not find graphics queue");
    ASSERT_MSG(transferQueueInfo.queueIndex != UINT32_MAX, "Err: did not find transfer queue");

    const float queuePriority = 1.0f;
    const uint32_t maxSupportedQueueCount = 3;
    uint32_t currentQueueCount = 0;

    VkDeviceQueueCreateInfo queueCreateInfo[maxSupportedQueueCount] = {};
    if (presentQueueInfo.queueIndex != graphicsQueueInfo.queueIndex) {
        queueCreateInfo[currentQueueCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo[currentQueueCount].queueFamilyIndex = presentQueueInfo.queueIndex;
        queueCreateInfo[currentQueueCount].queueCount = 1;
        queueCreateInfo[currentQueueCount].pQueuePriorities = &queuePriority;
        ++currentQueueCount;
    }

    queueCreateInfo[currentQueueCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo[currentQueueCount].queueFamilyIndex = graphicsQueueInfo.queueIndex;
    queueCreateInfo[currentQueueCount].queueCount = 1;
    queueCreateInfo[currentQueueCount].pQueuePriorities = &queuePriority;
    ++currentQueueCount;

    queueCreateInfo[currentQueueCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo[currentQueueCount].queueFamilyIndex = transferQueueInfo.queueIndex;
    queueCreateInfo[currentQueueCount].queueCount = 1;
    queueCreateInfo[currentQueueCount].pQueuePriorities = &queuePriority;
    ++currentQueueCount;

    VkPhysicalDeviceFeatures2 deviceFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
    deviceFeatures.features.samplerAnisotropy = VK_TRUE;

    uint32_t deviceExtensionCount = 0;
    const uint32_t maxSupportedDeviceExtensions = 2;
    const char *enabledDeviceExtensions[maxSupportedDeviceExtensions];
    {
        enabledDeviceExtensions[deviceExtensionCount] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        deviceExtensionCount++;
    }

#if BEET_VK_COMPILE_VERSION_1_3
    const uint32_t runtimeVulkanVersion = g_vulkanProperties->selectedPhysicalDevice.apiVersion;
    if (runtimeVulkanVersion >= BEET_VK_API_VERSION_1_3) {
        enabledDeviceExtensions[deviceExtensionCount] = VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME;
        deviceExtensionCount++;
    }
#endif

    VkDeviceCreateInfo deviceCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    deviceCreateInfo.pNext = &deviceFeatures;
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = nullptr;
    deviceCreateInfo.enabledExtensionCount = deviceExtensionCount;
    deviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensions;
    deviceCreateInfo.queueCreateInfoCount = currentQueueCount;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfo;

    vkCreateDevice(g_gfxDevice->vkPhysicalDevice, &deviceCreateInfo, nullptr, &g_gfxDevice->vkDevice);
    ASSERT_MSG(g_gfxDevice->vkDevice, "Err: failed to create vkDevice");

    vkGetDeviceQueue(g_gfxDevice->vkDevice, presentQueueInfo.queueIndex, 0, &g_gfxDevice->vkPresentQueue);
    vkGetDeviceQueue(g_gfxDevice->vkDevice, graphicsQueueInfo.queueIndex, 0, &g_gfxDevice->vkGraphicsQueue);
    vkGetDeviceQueue(g_gfxDevice->vkDevice, transferQueueInfo.queueIndex, 0, &g_gfxDevice->vkTransferQueue);

    g_gfxDevice->presentQueueIndex = presentQueueInfo.queueIndex;
    g_gfxDevice->graphicsQueueIndex = graphicsQueueInfo.queueIndex;
    g_gfxDevice->transferQueueIndex = transferQueueInfo.queueIndex;

    ASSERT_MSG(g_gfxDevice->vkPresentQueue, "Err: failed to create present queue");
    ASSERT_MSG(g_gfxDevice->vkGraphicsQueue, "Err: failed to create graphics queue");
    ASSERT_MSG(g_gfxDevice->vkTransferQueue, "Err: failed to create transfer queue");

    delete[] queueFamilies;
    delete[] selectedPhysicalDeviceExtensions;
}

void gfx_cleanup_queues() {
    vkDestroyDevice(g_gfxDevice->vkDevice, nullptr);
    g_gfxDevice->vkGraphicsQueue = nullptr;
    g_gfxDevice->vkPresentQueue = nullptr;
    g_gfxDevice->vkTransferQueue = nullptr;
}

void gfx_create_command_pool() {
    //===graphics==============
    VkCommandPoolCreateInfo commandPoolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    commandPoolInfo.queueFamilyIndex = g_gfxDevice->graphicsQueueIndex;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkResult cmdPoolResult =
            vkCreateCommandPool(
                    g_gfxDevice->vkDevice,
                    &commandPoolInfo,
                    nullptr,
                    &g_gfxDevice->vkGraphicsCommandPool
            );
    ASSERT_MSG(cmdPoolResult == VK_SUCCESS, "Err: failed to create graphics command pool");

    VkCommandBufferAllocateInfo commandBufferInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    commandBufferInfo.commandPool = g_gfxDevice->vkGraphicsCommandPool;
    commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferInfo.commandBufferCount = BEET_VK_COMMAND_BUFFER_COUNT;

    VkResult cmdGraphicsBufferResult =
            vkAllocateCommandBuffers(
                    g_gfxDevice->vkDevice,
                    &commandBufferInfo,
                    g_gfxDevice->vkGraphicsCommandBuffers
            );
    ASSERT_MSG(cmdGraphicsBufferResult == VK_SUCCESS, "Err: failed to create graphics command buffer");

    VkFenceCreateInfo fenceInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (uint32_t i = 0; i < BEET_VK_COMMAND_BUFFER_COUNT; ++i) {
        VkResult fenceRes = vkCreateFence(
                g_gfxDevice->vkDevice,
                &fenceInfo,
                nullptr,
                &g_gfxDevice->vkGraphicsFences[i]
        );
        ASSERT_MSG(fenceRes == VK_SUCCESS, "Err: failed to create fence [%u]", i);
    }
    //===immediate (graphics)====

    VkResult fenceRes =
            vkCreateFence(
                    g_gfxDevice->vkDevice,
                    &fenceInfo,
                    nullptr,
                    &g_gfxDevice->vkImmediateFence
            );
    ASSERT_MSG(fenceRes == VK_SUCCESS, "Err: failed to create immediate fence");

    commandBufferInfo.commandBufferCount = 1;
    VkResult cmdImmediateBufferResult =
            vkAllocateCommandBuffers(
                    g_gfxDevice->vkDevice,
                    &commandBufferInfo,
                    &g_gfxDevice->vkImmediateCommandBuffer
            );
    ASSERT_MSG(cmdImmediateBufferResult == VK_SUCCESS, "Err: failed to create immediate command buffer");

    //===transfer================
    //TODO:GFX: transfer pool command buffer for async.
}

void gfx_cleanup_command_pool() {
    {
        vkFreeCommandBuffers(
                g_gfxDevice->vkDevice, g_gfxDevice->vkGraphicsCommandPool,
                BEET_VK_COMMAND_BUFFER_COUNT, g_gfxDevice->vkGraphicsCommandBuffers
        );
        for (uint32_t i = 0; i < BEET_VK_COMMAND_BUFFER_COUNT; ++i) {
            g_gfxDevice->vkGraphicsCommandBuffers[i] = VK_NULL_HANDLE;
        }

        vkFreeCommandBuffers(
                g_gfxDevice->vkDevice, g_gfxDevice->vkGraphicsCommandPool,
                1, &g_gfxDevice->vkImmediateCommandBuffer
        );
        g_gfxDevice->vkImmediateCommandBuffer = VK_NULL_HANDLE;
    }
    {
        for (uint32_t i = 0; i < BEET_VK_COMMAND_BUFFER_COUNT; ++i) {
            vkDestroyFence(g_gfxDevice->vkDevice, g_gfxDevice->vkGraphicsFences[i], nullptr);
            g_gfxDevice->vkGraphicsFences[i] = VK_NULL_HANDLE;
        }

        vkDestroyFence(g_gfxDevice->vkDevice, g_gfxDevice->vkImmediateFence, nullptr);
        g_gfxDevice->vkImmediateFence = VK_NULL_HANDLE;
    }
    {
        vkDestroyCommandPool(g_gfxDevice->vkDevice, g_gfxDevice->vkGraphicsCommandPool, nullptr);
        g_gfxDevice->vkGraphicsCommandPool = VK_NULL_HANDLE;
    }
}


void gfx_create_allocator() {
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = g_gfxDevice->vkPhysicalDevice;
    allocatorInfo.device = g_gfxDevice->vkDevice;
    allocatorInfo.instance = g_gfxDevice->vkInstance;
    vmaCreateAllocator(&allocatorInfo, &g_gfxDevice->vmaAllocator);
}

void gfx_cleanup_allocator() {
    vmaDestroyAllocator(g_gfxDevice->vmaAllocator);
}

void gfx_create_swapchain() {
    VkSurfaceFormatKHR selectedSurfaceFormat = select_surface_format();
    VkPresentModeKHR selectedPresentMode = select_present_mode();

    create_swapchain(selectedSurfaceFormat, selectedPresentMode);
    create_swapchain_image_views(selectedSurfaceFormat.format);

    VkFormat selectedDepthFormat = find_depth_format(VK_IMAGE_TILING_OPTIMAL);
    create_depth_buffer(selectedDepthFormat);

    gfx_create_lit_pipeline_layout();
    gfx_create_lit_renderpass(selectedSurfaceFormat.format, selectedDepthFormat);
    gfx_create_lit_framebuffer();
    gfx_create_lit_pipeline();

    gfx_create_font_pipeline_layout();
    gfx_create_font_renderpass(selectedSurfaceFormat.format, selectedDepthFormat);
    gfx_create_font_framebuffer();
    gfx_create_font_pipeline();

    create_semaphores();
}


void gfx_recreate_swapchain() {
    if (!query_has_valid_extent_size()) {
        return;
    }

    vkDeviceWaitIdle(g_gfxDevice->vkDevice);

    destroy_swapchain();
    gfx_create_swapchain();
}

void gfx_cleanup_inflight_data() {
    for (int i = 0; i < BEET_VK_COMMAND_BUFFER_COUNT; ++i) {
        const uint32_t cmdBufIndex = g_gfxDevice->nextCommandBufferIndex;

        VkCommandBuffer hCommandBuffer = g_gfxDevice->vkGraphicsCommandBuffers[cmdBufIndex];
        VkFence hCommandBufferExecutedFence = g_gfxDevice->vkGraphicsFences[cmdBufIndex];

        vkWaitForFences(g_gfxDevice->vkDevice, 1, &hCommandBufferExecutedFence, VK_TRUE, UINT64_MAX);
        vkResetFences(g_gfxDevice->vkDevice, 1, &hCommandBufferExecutedFence);

        vkResetCommandBuffer(hCommandBuffer, 0);

        uint32_t imageIndex = 0;
        VkResult res = vkAcquireNextImageKHR(
                g_gfxDevice->vkDevice,
                g_gfxDevice->vkSwapchain,
                UINT64_MAX,
                g_gfxDevice->vkSemaphoreImageAvailable,
                VK_NULL_HANDLE,
                &imageIndex
        );

        g_gfxDevice->nextCommandBufferIndex = ++g_gfxDevice->nextCommandBufferIndex % BEET_VK_COMMAND_BUFFER_COUNT;
    }
    vkDeviceWaitIdle(g_gfxDevice->vkDevice);
}

void gfx_cleanup_swapchain() {
    gfx_cleanup_inflight_data();
    vkDeviceWaitIdle(g_gfxDevice->vkDevice);
    destroy_swapchain();

    vkDestroySwapchainKHR(g_gfxDevice->vkDevice, g_gfxDevice->vkSwapchain, nullptr);
    g_gfxDevice->vkSwapchain = VK_NULL_HANDLE;
}

void gfx_next_frame() {
    g_gfxDevice->nextCommandBufferIndex = ++g_gfxDevice->nextCommandBufferIndex % BEET_VK_COMMAND_BUFFER_COUNT;
}

VkCommandBuffer gfx_graphics_command_buffer() {
    return g_gfxDevice->vkGraphicsCommandBuffers[g_gfxDevice->nextCommandBufferIndex];
}


void gfx_sync() {
    vkWaitForFences(
            g_gfxDevice->vkDevice,
            1, &g_gfxDevice->vkGraphicsFences[g_gfxDevice->nextCommandBufferIndex],
            VK_TRUE,
            UINT64_MAX
    );

    vkResetFences(
            g_gfxDevice->vkDevice,
            1,
            &g_gfxDevice->vkGraphicsFences[g_gfxDevice->nextCommandBufferIndex]
    );
}

void gfx_reset_graphics_command_buffer() {
    vkResetCommandBuffer(g_gfxDevice->vkGraphicsCommandBuffers[g_gfxDevice->nextCommandBufferIndex], 0);
}

void gfx_update(const double &deltaTime) {
    static double timePassed{};
    timePassed += deltaTime;

    VkResult res = acquire_next_swapchain_image();
    if (res == VK_ERROR_OUT_OF_DATE_KHR) {
        gfx_recreate_swapchain();
        return;
    } else if (res < 0) {
        ASSERT(res == VK_SUCCESS)
    }

    gfx_next_frame();
    gfx_sync();

    VkCommandBuffer cmdBuffer = gfx_graphics_command_buffer();
    gfx_reset_graphics_command_buffer();

    begin_command_recording(cmdBuffer);
    {
        gfx_lit_record_render_pass(cmdBuffer);
        gfx_font_record_render_pass(cmdBuffer);
    }
    end_command_recording(cmdBuffer);

    gfx_command_submit(cmdBuffer);

    preset_queue();
}




