//===defines=================
#include <gfx/gfx_interface.h>
#include <gfx/vulkan_platform_defines.h>

#include <shared/log.h>
#include <shared/assert.h>
#include <shared/bit_utils.h>

#include <filesystem>
#include <iostream>
#include <fstream>
#include <unordered_map>
//
#include <math/utilities.h>
#include <math/mat4.h>
#include <math/vec3.h>

#define VMA_IMPLEMENTATION

#include <vma/vk_mem_alloc.h>

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
    VkDevice vkDevice{};
    VmaAllocator vmaAllocator{};

    VkExtent2D vkExtent{};

    VkQueue vkGraphicsQueue{};
    VkQueue vkPresentQueue{};
    uint32_t graphicsQueueIndex{};
    uint32_t presentQueueIndex{};
    VkQueue vkTransferQueue{}; // unused
    uint32_t transferQueueIndex{}; // unused

    VkSwapchainKHR vkSwapchain{};
    VkImageView *vkSwapchainImageViews{};
    uint32_t swapchainImageViewCount{};
    VkImage depthImage{};
    VmaAllocation depthAllocation{};
    VkImageView depthImageView{};

    VkFramebuffer *vkFrameBuffer{};
    uint32_t frameBufferCount{};
    uint32_t swapchainImageIndex{};

    VkSemaphore vkSemaphoreImageAvailable{};
    VkSemaphore vkSemaphoreRenderFinished{};

    VkCommandPool vkGraphicsCommandPool{};
    VkCommandBuffer vkGraphicsCommandBuffers[BEET_VK_COMMAND_BUFFER_COUNT]{};
    VkFence vkGraphicsFences[BEET_VK_COMMAND_BUFFER_COUNT]{};
    uint32_t nextCommandBufferIndex{};


    VkFence vkImmediateFence{};
    VkCommandBuffer vkImmediateCommandBuffer{};

    VkDebugUtilsMessengerEXT vkDebugUtilsMessengerExt = VK_NULL_HANDLE;

};

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

struct TextureSamplers {
    VkSampler linearSampler{};
    VkSampler pointSampler{};
};

TextureSamplers *g_textureSamplers;

struct VulkanFallbacks {
    VkImage imageTexture;
    VmaAllocation imageAllocation;
    VkImageView imageView;

    VkBuffer vertexBuffer;
    VmaAllocation vertexAllocation;

    VkBuffer indexBuffer;
    VmaAllocation indexAllocation;
    uint32_t vertexCount;
    uint32_t indexCount;

    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;

    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;
    VkPipeline pipeline;
};

VulkanFallbacks g_vulkanFallbacks;

struct UserArguments {
    uint32_t selectedPhysicalDeviceIndex{};
    bool vsync{true};
};

UserArguments *g_userArguments;

//===struct forward defs=====
struct UniformBufferObject {
    mat4 mvp;
};

struct Vertex {
    float pos[3];
    float color[3];
    float texCoord[2];
};

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

void begin_command_immediate_recording() {
    VkCommandBufferBeginInfo cmdBufBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    cmdBufBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(g_gfxDevice->vkImmediateCommandBuffer, &cmdBufBeginInfo);
}

void end_immediate_command_recording() {
    vkEndCommandBuffer(g_gfxDevice->vkImmediateCommandBuffer);

    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &g_gfxDevice->vkImmediateCommandBuffer;

    //TODO:GFX replace immediate submit with transfer immediate submit.
    vkQueueSubmit(g_gfxDevice->vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(g_gfxDevice->vkGraphicsQueue);
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

void load_shader_binary(const char *path, char **out_data, size_t &out_fileSize) {
    std::ifstream file{path, std::ios::ate | std::ios::binary};

    if (!file.is_open()) {
        log_warning("failed to find path: %s\n", path);
        out_fileSize = 0;
    }

    out_fileSize = static_cast<size_t>(file.tellg());

    *out_data = new char[out_fileSize]();

    file.seekg(0);
    file.read(*out_data, out_fileSize);
    file.close();
}

void destroy_swapchain() {
    {
        vkDestroySemaphore(g_gfxDevice->vkDevice, g_gfxDevice->vkSemaphoreImageAvailable, nullptr);
        g_gfxDevice->vkSemaphoreImageAvailable = VK_NULL_HANDLE;

        vkDestroySemaphore(g_gfxDevice->vkDevice, g_gfxDevice->vkSemaphoreRenderFinished, nullptr);
        g_gfxDevice->vkSemaphoreRenderFinished = VK_NULL_HANDLE;
    }
    {
        for (int i = 0; i < g_gfxDevice->frameBufferCount; ++i) {
            vkDestroyFramebuffer(g_gfxDevice->vkDevice, g_gfxDevice->vkFrameBuffer[i], nullptr);
        }
        g_gfxDevice->frameBufferCount = 0;
        delete[] g_gfxDevice->vkFrameBuffer;
        g_gfxDevice->vkFrameBuffer = nullptr;
    }
    {
        vkDestroyPipeline(g_gfxDevice->vkDevice, g_vulkanFallbacks.pipeline, nullptr);
        g_vulkanFallbacks.pipeline = VK_NULL_HANDLE;
    }
    {
        vkDestroyRenderPass(g_gfxDevice->vkDevice, g_vulkanFallbacks.renderPass, nullptr);
        g_vulkanFallbacks.renderPass = VK_NULL_HANDLE;
    }
    {
        vkDestroyPipelineLayout(g_gfxDevice->vkDevice, g_vulkanFallbacks.pipelineLayout, nullptr);
        g_vulkanFallbacks.pipelineLayout = VK_NULL_HANDLE;
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
    for (int i = 0; i < presentModeCount; ++i) {
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
        for (int i = 0; i < surfaceFormatsCount; ++i) {
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

void create_fallback_pipeline_layout() {
    VkPushConstantRange pushConstantRanges[1]{};
    pushConstantRanges[0].offset = 0;
    pushConstantRanges[0].size = sizeof(UniformBufferObject);
    pushConstantRanges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    const uint32_t descriptorSetLayoutsCount = 1;
    VkDescriptorSetLayout descriptorSetLayouts[descriptorSetLayoutsCount] = {g_vulkanFallbacks.descriptorSetLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = descriptorSetLayoutsCount;
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges;
    vkCreatePipelineLayout(g_gfxDevice->vkDevice, &pipelineLayoutInfo, nullptr, &g_vulkanFallbacks.pipelineLayout);
}

void create_fallback_renderpass(const VkFormat &selectedSurfaceFormat, const VkFormat &selectedDepthFormat) {
    if (g_vulkanFallbacks.renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(g_gfxDevice->vkDevice, g_vulkanFallbacks.renderPass, nullptr);
        g_vulkanFallbacks.renderPass = VK_NULL_HANDLE;
    }

    VkAttachmentDescription attachments[2];
    ZeroMemory(attachments, sizeof(attachments));

    attachments[0].format = selectedSurfaceFormat;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    attachments[1].format = selectedDepthFormat;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthStencilAttachmentRef = {};
    depthStencilAttachmentRef.attachment = 1;
    depthStencilAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDesc = {};
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colorAttachmentRef;
    subpassDesc.pDepthStencilAttachment = &depthStencilAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    renderPassInfo.attachmentCount = (uint32_t) _countof(attachments);
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDesc;
    renderPassInfo.dependencyCount = 0;
    vkCreateRenderPass(g_gfxDevice->vkDevice, &renderPassInfo, nullptr, &g_vulkanFallbacks.renderPass);
}

void create_fallback_pipeline() {
    {
        char *vertShaderCode = nullptr;
        size_t vertShaderCodeSize{};
        load_shader_binary("../res/shaders/fallback/fallback.vert.spv", &vertShaderCode, vertShaderCodeSize);
        ASSERT_MSG(vertShaderCode != nullptr, "Err: failed to load vert shader");
        ASSERT_MSG(vertShaderCodeSize != 0, "Err: failed to load vert shader");

        VkShaderModuleCreateInfo shaderModuleInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        shaderModuleInfo.codeSize = vertShaderCodeSize;
        shaderModuleInfo.pCode = (const uint32_t *) vertShaderCode;
        VkShaderModule vertShader = VK_NULL_HANDLE;
        vkCreateShaderModule(g_gfxDevice->vkDevice, &shaderModuleInfo, nullptr, &vertShader);

        char *fragShaderCode = nullptr;
        size_t fragShaderCodeSize{};
        load_shader_binary("../res/shaders/fallback/fallback.frag.spv", &fragShaderCode, fragShaderCodeSize);
        ASSERT_MSG(fragShaderCode != nullptr, "Err: failed to load frag shader");
        ASSERT_MSG(fragShaderCodeSize != 0, "Err: failed to load frag shader");

        shaderModuleInfo.codeSize = fragShaderCodeSize;
        shaderModuleInfo.pCode = (const uint32_t *) fragShaderCode;
        VkShaderModule fragShader = VK_NULL_HANDLE;
        vkCreateShaderModule(g_gfxDevice->vkDevice, &shaderModuleInfo, nullptr, &fragShader);

        VkPipelineShaderStageCreateInfo vertPipelineShaderStageInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        vertPipelineShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertPipelineShaderStageInfo.module = vertShader;
        vertPipelineShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragPipelineShaderStageInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        fragPipelineShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragPipelineShaderStageInfo.module = fragShader;
        fragPipelineShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo pipelineShaderStageInfos[] = {
                vertPipelineShaderStageInfo,
                fragPipelineShaderStageInfo
        };

        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription attributeDescriptions[3]{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        pipelineVertexInputStateInfo.vertexBindingDescriptionCount = 1;
        pipelineVertexInputStateInfo.pVertexBindingDescriptions = &bindingDescription;
        pipelineVertexInputStateInfo.vertexAttributeDescriptionCount = _countof(attributeDescriptions);
        pipelineVertexInputStateInfo.pVertexAttributeDescriptions = attributeDescriptions;

        VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        pipelineInputAssemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        pipelineInputAssemblyStateInfo.primitiveRestartEnable = VK_TRUE;

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) g_gfxDevice->vkExtent.width;
        viewport.height = (float) g_gfxDevice->vkExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent = g_gfxDevice->vkExtent;

        VkPipelineViewportStateCreateInfo pipelineViewportStateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        pipelineViewportStateInfo.viewportCount = 1;
        pipelineViewportStateInfo.pViewports = &viewport;
        pipelineViewportStateInfo.scissorCount = 1;
        pipelineViewportStateInfo.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        pipelineRasterizationStateInfo.depthClampEnable = VK_FALSE;
        pipelineRasterizationStateInfo.rasterizerDiscardEnable = VK_FALSE;
        pipelineRasterizationStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        pipelineRasterizationStateInfo.lineWidth = 1.0f;
        pipelineRasterizationStateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        pipelineRasterizationStateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        pipelineRasterizationStateInfo.depthBiasEnable = VK_FALSE;
        pipelineRasterizationStateInfo.depthBiasConstantFactor = 0.0f;
        pipelineRasterizationStateInfo.depthBiasClamp = 0.0f;
        pipelineRasterizationStateInfo.depthBiasSlopeFactor = 0.0f;

        VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        pipelineMultisampleStateInfo.sampleShadingEnable = VK_FALSE;
        pipelineMultisampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        pipelineMultisampleStateInfo.minSampleShading = 1.0f;
        pipelineMultisampleStateInfo.pSampleMask = nullptr;
        pipelineMultisampleStateInfo.alphaToCoverageEnable = VK_FALSE;
        pipelineMultisampleStateInfo.alphaToOneEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState = {};
        pipelineColorBlendAttachmentState.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT |
                VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT;
        pipelineColorBlendAttachmentState.blendEnable = VK_FALSE;
        pipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        pipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        pipelineColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        pipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        pipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        pipelineColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        pipelineColorBlendStateInfo.logicOpEnable = VK_FALSE;
        pipelineColorBlendStateInfo.logicOp = VK_LOGIC_OP_COPY;
        pipelineColorBlendStateInfo.attachmentCount = 1;
        pipelineColorBlendStateInfo.pAttachments = &pipelineColorBlendAttachmentState;

        VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        depthStencilStateInfo.depthTestEnable = VK_TRUE;
        depthStencilStateInfo.depthWriteEnable = VK_TRUE;
        depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilStateInfo.stencilTestEnable = VK_FALSE;

        VkGraphicsPipelineCreateInfo pipelineInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = pipelineShaderStageInfos;
        pipelineInfo.pVertexInputState = &pipelineVertexInputStateInfo;
        pipelineInfo.pInputAssemblyState = &pipelineInputAssemblyStateInfo;
        pipelineInfo.pViewportState = &pipelineViewportStateInfo;
        pipelineInfo.pRasterizationState = &pipelineRasterizationStateInfo;
        pipelineInfo.pMultisampleState = &pipelineMultisampleStateInfo;
        pipelineInfo.pDepthStencilState = &depthStencilStateInfo;
        pipelineInfo.pColorBlendState = &pipelineColorBlendStateInfo;
        pipelineInfo.pDynamicState = nullptr;
        pipelineInfo.layout = g_vulkanFallbacks.pipelineLayout;
        pipelineInfo.renderPass = g_vulkanFallbacks.renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        vkCreateGraphicsPipelines(
                g_gfxDevice->vkDevice,
                VK_NULL_HANDLE,
                1,
                &pipelineInfo,
                nullptr,
                &g_vulkanFallbacks.pipeline
        );

        vkDestroyShaderModule(g_gfxDevice->vkDevice, fragShader, nullptr);
        vkDestroyShaderModule(g_gfxDevice->vkDevice, vertShader, nullptr);

        delete[] vertShaderCode;
        delete[] fragShaderCode;
    }
}

void create_framebuffer() {
    g_gfxDevice->frameBufferCount = g_gfxDevice->swapchainImageViewCount;
    g_gfxDevice->vkFrameBuffer = new VkFramebuffer[g_gfxDevice->frameBufferCount];

    for (size_t i = 0; i < g_gfxDevice->frameBufferCount; ++i) {
        const uint32_t attachmentsCount = 2;
        VkImageView attachments[attachmentsCount] = {g_gfxDevice->vkSwapchainImageViews[i], g_gfxDevice->depthImageView};

        VkFramebufferCreateInfo framebufferInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        framebufferInfo.renderPass = g_vulkanFallbacks.renderPass;
        framebufferInfo.attachmentCount = attachmentsCount;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = g_gfxDevice->vkExtent.width;
        framebufferInfo.height = g_gfxDevice->vkExtent.height;
        framebufferInfo.layers = 1;
        vkCreateFramebuffer(g_gfxDevice->vkDevice, &framebufferInfo, nullptr, &g_gfxDevice->vkFrameBuffer[i]);
    }
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
    g_textureSamplers = new TextureSamplers;
}

void gfx_cleanup() {
    {
        delete g_textureSamplers;
        g_textureSamplers = nullptr;
    }
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
    if (BEET_MAX_VK_API_VERSION == VK_API_VERSION_1_3) {
        enabledDeviceExtensions[deviceExtensionCount] = VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME;
        deviceExtensionCount++;
    }

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


void gfx_create_samplers() {
    {

        VkSamplerCreateInfo samplerInfo = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = 16;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = FLT_MAX;
        VkResult linearResult = vkCreateSampler(g_gfxDevice->vkDevice, &samplerInfo, nullptr,
                                                &g_textureSamplers->linearSampler);
        ASSERT_MSG(linearResult == VK_SUCCESS, "Err: failed to create linear sampler");
    }
    {

        VkSamplerCreateInfo samplerInfo = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        samplerInfo.magFilter = VK_FILTER_NEAREST;
        samplerInfo.minFilter = VK_FILTER_NEAREST;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = 16;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = FLT_MAX;
        VkResult pointResult = vkCreateSampler(g_gfxDevice->vkDevice, &samplerInfo, nullptr,
                                               &g_textureSamplers->pointSampler);
        ASSERT_MSG(pointResult == VK_SUCCESS, "Err: failed to create linear sampler");
    }
}

void gfx_cleanup_samplers() {
    vkDestroySampler(g_gfxDevice->vkDevice, g_textureSamplers->linearSampler, nullptr);
    g_textureSamplers->linearSampler = VK_NULL_HANDLE;
    vkDestroySampler(g_gfxDevice->vkDevice, g_textureSamplers->pointSampler, nullptr);
    g_textureSamplers->pointSampler = VK_NULL_HANDLE;
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

void gfx_create_fallback_texture() {
    ASSERT_MSG(g_gfxDevice->vmaAllocator, "Err: vma allocator hasn't been created yet");

    const size_t RGBA8_SIZE = sizeof(uint32_t);
    const uint32_t sizeX = 16;
    const uint32_t sizeY = 16;
    const VkDeviceSize imageSize = sizeX * sizeY * RGBA8_SIZE;

    //A8 R8 G8 B8
    const uint32_t colourBlack = 0xFF000000;
    const uint32_t colourMagenta = 0xFFFF00FF;

    uint32_t rawImageData[sizeX * sizeY];
    for (uint32_t i = 0; i < sizeX; ++i) {
        for (uint32_t j = 0; j < sizeY; ++j) {
            rawImageData[i * sizeY + j] = (i + j) % 2 ? colourBlack : colourMagenta;
        }
    }

    VkBufferCreateInfo stagingBufInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    stagingBufInfo.size = imageSize;
    stagingBufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo stagingBufAllocCreateInfo = {};
    stagingBufAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    stagingBufAllocCreateInfo.flags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VkBuffer stagingBuf = VK_NULL_HANDLE;
    VmaAllocation stagingBufAlloc = VK_NULL_HANDLE;
    VmaAllocationInfo stagingBufAllocInfo = {};

    VkResult createBufferRes = vmaCreateBuffer(
            g_gfxDevice->vmaAllocator,
            &stagingBufInfo,
            &stagingBufAllocCreateInfo,
            &stagingBuf,
            &stagingBufAlloc,
            &stagingBufAllocInfo
    );
    ASSERT_MSG(createBufferRes == VK_SUCCESS, "Err: Failed to create staging buffers");

    memcpy(stagingBufAllocInfo.pMappedData, rawImageData, imageSize);

    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = sizeX;
    imageInfo.extent.height = sizeY;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    VmaAllocationCreateInfo imageAllocCreateInfo = {};
    imageAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

    VkResult imageRes = vmaCreateImage(
            g_gfxDevice->vmaAllocator,
            &imageInfo,
            &imageAllocCreateInfo,
            &g_vulkanFallbacks.imageTexture,
            &g_vulkanFallbacks.imageAllocation,
            nullptr
    );
    ASSERT_MSG(imageRes == VK_SUCCESS, "Err: failed to allocate image");

    begin_command_immediate_recording();
    {
        VkImageMemoryBarrier imgMemBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        imgMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imgMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imgMemBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imgMemBarrier.subresourceRange.baseMipLevel = 0;
        imgMemBarrier.subresourceRange.levelCount = 1;
        imgMemBarrier.subresourceRange.baseArrayLayer = 0;
        imgMemBarrier.subresourceRange.layerCount = 1;
        imgMemBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imgMemBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imgMemBarrier.image = g_vulkanFallbacks.imageTexture;
        imgMemBarrier.srcAccessMask = 0;
        imgMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        vkCmdPipelineBarrier(
                g_gfxDevice->vkImmediateCommandBuffer,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1,
                &imgMemBarrier
        );

        VkBufferImageCopy region = {};
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.layerCount = 1;
        region.imageExtent.width = sizeX;
        region.imageExtent.height = sizeY;
        region.imageExtent.depth = 1;

        vkCmdCopyBufferToImage(
                g_gfxDevice->vkImmediateCommandBuffer,
                stagingBuf,
                g_vulkanFallbacks.imageTexture,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &region
        );

        imgMemBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imgMemBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imgMemBarrier.image = g_vulkanFallbacks.imageTexture;
        imgMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imgMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(
                g_gfxDevice->vkImmediateCommandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0,
                0,
                nullptr,
                0,
                nullptr,
                1,
                &imgMemBarrier
        );
    }
    end_immediate_command_recording();

    vmaDestroyBuffer(g_gfxDevice->vmaAllocator, stagingBuf, stagingBufAlloc);

    VkImageViewCreateInfo textureImageViewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    textureImageViewInfo.image = g_vulkanFallbacks.imageTexture;
    textureImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    textureImageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    textureImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    textureImageViewInfo.subresourceRange.baseMipLevel = 0;
    textureImageViewInfo.subresourceRange.levelCount = 1;
    textureImageViewInfo.subresourceRange.baseArrayLayer = 0;
    textureImageViewInfo.subresourceRange.layerCount = 1;

    VkResult imageViewRes = vkCreateImageView(
            g_gfxDevice->vkDevice,
            &textureImageViewInfo,
            nullptr,
            &g_vulkanFallbacks.imageView
    );
    ASSERT_MSG(imageViewRes == VK_SUCCESS, "Err: failed to create image view");
}

void gfx_cleanup_fallback_texture() {
    vkDestroyImageView(g_gfxDevice->vkDevice, g_vulkanFallbacks.imageView, nullptr);
    g_vulkanFallbacks.imageView = VK_NULL_HANDLE;

    vmaDestroyImage(g_gfxDevice->vmaAllocator, g_vulkanFallbacks.imageTexture, g_vulkanFallbacks.imageAllocation);
    g_vulkanFallbacks.imageTexture = VK_NULL_HANDLE;
}

void gfx_create_fallback_mesh() {
    ASSERT_MSG(g_gfxDevice->vmaAllocator, "Err: vma allocator hasn't been created yet");

    const uint32_t vertexCount = 24;
    static Vertex vertices[vertexCount] = {
            //===POS================//===COLOUR===========//===UV===
            // -x
            {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{-1.0f, -1.0f, 1.0f},  {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{-1.0f, 1.0f,  -1.0f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-1.0f, 1.0f,  1.0f},  {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            // +x
            {{1.0f,  -1.0f, 1.0f},  {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{1.0f,  -1.0f, -1.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{1.0f,  1.0f,  1.0f},  {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{1.0f,  1.0f,  -1.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            // -z
            {{1.0f,  -1.0f, -1.0f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{1.0f,  1.0f,  -1.0f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-1.0f, 1.0f,  -1.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            // +z
            {{-1.0f, -1.0f, 1.0f},  {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{1.0f,  -1.0f, 1.0f},  {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{-1.0f, 1.0f,  1.0f},  {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{1.0f,  1.0f,  1.0f},  {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            // -y
            {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{1.0f,  -1.0f, -1.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{-1.0f, -1.0f, 1.0f},  {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{1.0f,  -1.0f, 1.0f},  {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            // +y
            {{1.0f,  1.0f,  -1.0f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{-1.0f, 1.0f,  -1.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{1.0f,  1.0f,  1.0f},  {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-1.0f, 1.0f,  1.0f},  {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    };

    const uint32_t indexCount = 30;
    static uint16_t indices[indexCount] = {
            0, 1, 2, 3, USHRT_MAX,
            4, 5, 6, 7, USHRT_MAX,
            8, 9, 10, 11, USHRT_MAX,
            12, 13, 14, 15, USHRT_MAX,
            16, 17, 18, 19, USHRT_MAX,
            20, 21, 22, 23, USHRT_MAX,
    };

    size_t vertexBufferSize = sizeof(Vertex) * vertexCount;
    size_t indexBufferSize = sizeof(uint16_t) * indexCount;
    g_vulkanFallbacks.indexCount = indexCount;

    // Create vertex buffer

    VkBufferCreateInfo vbInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    vbInfo.size = vertexBufferSize;
    vbInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vbInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo vbAllocCreateInfo = {};
    vbAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    vbAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VkBuffer stagingVertexBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingVertexBufferAlloc = VK_NULL_HANDLE;
    VmaAllocationInfo stagingVertexBufferAllocInfo = {};
    VkResult vertexStagingRes = vmaCreateBuffer(
            g_gfxDevice->vmaAllocator,
            &vbInfo, &vbAllocCreateInfo,
            &stagingVertexBuffer,
            &stagingVertexBufferAlloc,
            &stagingVertexBufferAllocInfo
    );
    ASSERT_MSG(vertexStagingRes == VK_SUCCESS, "Err: failed to create staging vertex buffer");

    memcpy(stagingVertexBufferAllocInfo.pMappedData, vertices, vertexBufferSize);

    vbInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vbAllocCreateInfo.flags = 0;
    VkResult vertexBufferRes = vmaCreateBuffer(
            g_gfxDevice->vmaAllocator,
            &vbInfo,
            &vbAllocCreateInfo,
            &g_vulkanFallbacks.vertexBuffer,
            &g_vulkanFallbacks.vertexAllocation,
            nullptr
    );
    ASSERT_MSG(vertexBufferRes == VK_SUCCESS, "Err: failed to create vertex buffer");
    // Create index buffer

    VkBufferCreateInfo ibInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    ibInfo.size = indexBufferSize;
    ibInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    ibInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo ibAllocCreateInfo = {};
    ibAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    ibAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VkBuffer stagingIndexBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingIndexBufferAlloc = VK_NULL_HANDLE;
    VmaAllocationInfo stagingIndexBufferAllocInfo = {};
    VkResult stagingIndexBufferRes = vmaCreateBuffer(
            g_gfxDevice->vmaAllocator,
            &ibInfo,
            &ibAllocCreateInfo,
            &stagingIndexBuffer,
            &stagingIndexBufferAlloc,
            &stagingIndexBufferAllocInfo
    );
    ASSERT_MSG(stagingIndexBufferRes == VK_SUCCESS, "Err: failed to create index staging buffer");
    memcpy(stagingIndexBufferAllocInfo.pMappedData, indices, indexBufferSize);

    // No need to flush stagingIndexBuffer memory because CPU_ONLY memory is always HOST_COHERENT.

    ibInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    ibAllocCreateInfo.flags = 0;
    VkResult indexBufferRes = vmaCreateBuffer(
            g_gfxDevice->vmaAllocator,
            &ibInfo,
            &ibAllocCreateInfo,
            &g_vulkanFallbacks.indexBuffer,
            &g_vulkanFallbacks.indexAllocation,
            nullptr
    );
    ASSERT_MSG(indexBufferRes == VK_SUCCESS, "Err: failed to create index staging buffer");

    // Copy buffers

    begin_command_immediate_recording();
    {
        VkBufferCopy vbCopyRegion = {};
        vbCopyRegion.srcOffset = 0;
        vbCopyRegion.dstOffset = 0;
        vbCopyRegion.size = vbInfo.size;
        vkCmdCopyBuffer(
                g_gfxDevice->vkImmediateCommandBuffer,
                stagingVertexBuffer,
                g_vulkanFallbacks.vertexBuffer,
                1,
                &vbCopyRegion
        );

        VkBufferCopy ibCopyRegion = {};
        ibCopyRegion.srcOffset = 0;
        ibCopyRegion.dstOffset = 0;
        ibCopyRegion.size = ibInfo.size;
        vkCmdCopyBuffer(
                g_gfxDevice->vkImmediateCommandBuffer,
                stagingIndexBuffer,
                g_vulkanFallbacks.indexBuffer,
                1,
                &ibCopyRegion
        );
    }
    end_immediate_command_recording();

    vmaDestroyBuffer(g_gfxDevice->vmaAllocator, stagingIndexBuffer, stagingIndexBufferAlloc);
    vmaDestroyBuffer(g_gfxDevice->vmaAllocator, stagingVertexBuffer, stagingVertexBufferAlloc);
}

void gfx_cleanup_fallback_mesh() {

    vmaDestroyBuffer(g_gfxDevice->vmaAllocator, g_vulkanFallbacks.indexBuffer,
                     g_vulkanFallbacks.indexAllocation);
    g_vulkanFallbacks.indexBuffer = VK_NULL_HANDLE;

    vmaDestroyBuffer(g_gfxDevice->vmaAllocator, g_vulkanFallbacks.vertexBuffer,
                     g_vulkanFallbacks.vertexAllocation);
    g_vulkanFallbacks.vertexBuffer = VK_NULL_HANDLE;
}

void gfx_create_fallback_descriptors() {
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    descriptorSetLayoutInfo.bindingCount = 1;
    descriptorSetLayoutInfo.pBindings = &samplerLayoutBinding;
    VkResult descriptorLayoutRes = vkCreateDescriptorSetLayout(g_gfxDevice->vkDevice, &descriptorSetLayoutInfo, nullptr,
                                                               &g_vulkanFallbacks.descriptorSetLayout);
    ASSERT_MSG(descriptorLayoutRes == VK_SUCCESS, "Err: failed to create descriptor set layout");
    // Create descriptor pool

    VkDescriptorPoolSize descriptorPoolSizes[2];
    ZeroMemory(descriptorPoolSizes, sizeof(descriptorPoolSizes));
    descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorPoolSizes[0].descriptorCount = 1;
    descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorPoolSizes[1].descriptorCount = 1;

    VkDescriptorPoolCreateInfo descriptorPoolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    descriptorPoolInfo.poolSizeCount = (uint32_t) _countof(descriptorPoolSizes);
    descriptorPoolInfo.pPoolSizes = descriptorPoolSizes;
    descriptorPoolInfo.maxSets = 1;
    VkResult descriptorPoolRes = vkCreateDescriptorPool(g_gfxDevice->vkDevice, &descriptorPoolInfo, nullptr,
                                                        &g_vulkanFallbacks.descriptorPool);
    ASSERT_MSG(descriptorPoolRes == VK_SUCCESS, "Err: failed to create descriptor pool")
    // Create descriptor set layout

    VkDescriptorSetLayout descriptorSetLayouts[] = {g_vulkanFallbacks.descriptorSetLayout};
    VkDescriptorSetAllocateInfo descriptorSetInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    descriptorSetInfo.descriptorPool = g_vulkanFallbacks.descriptorPool;
    descriptorSetInfo.descriptorSetCount = 1;
    descriptorSetInfo.pSetLayouts = descriptorSetLayouts;
    VkResult allocateDescriptorRes =
            vkAllocateDescriptorSets(g_gfxDevice->vkDevice, &descriptorSetInfo, &g_vulkanFallbacks.descriptorSet);
    ASSERT_MSG(allocateDescriptorRes == VK_SUCCESS, "Err: failed to allocate descriptor set");

    VkDescriptorImageInfo descriptorImageInfo = {};
    descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    descriptorImageInfo.imageView = g_vulkanFallbacks.imageView;
    descriptorImageInfo.sampler = g_textureSamplers->pointSampler;

    VkWriteDescriptorSet writeDescriptorSet = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    writeDescriptorSet.dstSet = g_vulkanFallbacks.descriptorSet;
    writeDescriptorSet.dstBinding = 1;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.pImageInfo = &descriptorImageInfo;

    vkUpdateDescriptorSets(g_gfxDevice->vkDevice, 1, &writeDescriptorSet, 0, nullptr);
}

void gfx_cleanup_fallback_descriptors() {

    vkDestroyDescriptorPool(g_gfxDevice->vkDevice, g_vulkanFallbacks.descriptorPool, nullptr);
    g_vulkanFallbacks.descriptorPool = VK_NULL_HANDLE;

    vkDestroyDescriptorSetLayout(g_gfxDevice->vkDevice, g_vulkanFallbacks.descriptorSetLayout, nullptr);
    g_vulkanFallbacks.descriptorSetLayout = VK_NULL_HANDLE;
}

void gfx_create_swapchain() {
    VkSurfaceFormatKHR selectedSurfaceFormat = select_surface_format();
    VkPresentModeKHR selectedPresentMode = select_present_mode();

    create_swapchain(selectedSurfaceFormat, selectedPresentMode);
    create_swapchain_image_views(selectedSurfaceFormat.format);

    VkFormat selectedDepthFormat = find_depth_format(VK_IMAGE_TILING_OPTIMAL);
    create_depth_buffer(selectedDepthFormat);

    create_fallback_pipeline_layout();
    create_fallback_renderpass(selectedSurfaceFormat.format, selectedDepthFormat);
    create_fallback_pipeline();

    create_framebuffer();
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
    log_info("%f\n", timePassed);

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
        // Record geometry pass

        const uint32_t clearValueCount = 2;
        VkClearValue clearValues[clearValueCount]{};
        clearValues[0].color = {{0.5f, 0.092, 0.167f, 1.0f}};
        clearValues[1].depthStencil.depth = 1.0f;

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = g_vulkanFallbacks.renderPass;
        renderPassBeginInfo.framebuffer = g_gfxDevice->vkFrameBuffer[g_gfxDevice->swapchainImageIndex];
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent = g_gfxDevice->vkExtent;
        renderPassBeginInfo.clearValueCount = clearValueCount;
        renderPassBeginInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
            vkCmdBindPipeline(
                    cmdBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    g_vulkanFallbacks.pipeline
            );

            mat4 view = mat4f_look_at({0.f, 0.f, 0.f}, {0.f, -2.f, 4.f}, {0.f, 1.f, 0.f});
            mat4 proj = mat4f_perspective(as_radians(90.0), (float) g_gfxDevice->vkExtent.width / (float) g_gfxDevice->vkExtent.height, 0.1f, 1000.f);
            mat4 viewProj = view * proj;

            vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_vulkanFallbacks.pipelineLayout, 0, 1,
                                    &g_vulkanFallbacks.descriptorSet, 0, nullptr);

            mat4 model = mat4f_rotation_y((float) timePassed);

            UniformBufferObject ubo = {};
            ubo.mvp = model * viewProj;
            vkCmdPushConstants(cmdBuffer, g_vulkanFallbacks.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                               sizeof(UniformBufferObject),
                               &ubo);

            VkBuffer vertexBuffers[] = {g_vulkanFallbacks.vertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(cmdBuffer, g_vulkanFallbacks.indexBuffer, 0, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(cmdBuffer, g_vulkanFallbacks.indexCount, 1, 0, 0, 0);
        }
        vkCmdEndRenderPass(cmdBuffer);
    }
    end_command_recording(cmdBuffer);

    gfx_command_submit(cmdBuffer);

    preset_queue();
}




