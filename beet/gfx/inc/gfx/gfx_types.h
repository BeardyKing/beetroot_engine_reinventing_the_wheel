#ifndef BEETROOT_GFX_TYPES_H
#define BEETROOT_GFX_TYPES_H

#include <vulkan/vulkan_core.h>
#include <vma/vk_mem_alloc.h>

#include <gfx/vulkan_platform_defines.h>

#include <math/mat4.h>
#include <math/vec3.h>
#include <math/vec2.h>


struct UniformBufferObject {
    mat4 mvp;
};

struct Vertex {
    vec3f pos;
    vec3f color;
    vec2f texCoord;
};

struct GfxTexture{
    VkImage imageTexture;
    VmaAllocation imageAllocation;
    VkImageView imageView;
};

struct GfxRenderPass{
    VkFramebuffer* vkFramebuffer;
    uint32_t frameBufferCount;
    VkRenderPass vkRenderPass;
    uint32_t width;
    uint32_t weight;
};

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

    //TODO:GFX refactor as GfxTexture
    VkImage depthImage{};
    VmaAllocation depthAllocation{};
    VkImageView depthImageView{};

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




#endif //BEETROOT_GFX_TYPES_H
