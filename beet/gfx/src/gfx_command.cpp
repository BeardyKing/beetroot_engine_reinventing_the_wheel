#include <gfx/gfx_command.h>
#include <gfx/gfx_types.h>
#include <gfx/gfx_samplers.h>

#include <shared/assert.h>
#include <shared/log.h>

extern struct GfxDevice *g_gfxDevice;

void gfx_command_begin_immediate_recording() {
    VkCommandBufferBeginInfo cmdBufBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    cmdBufBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(g_gfxDevice->vkImmediateCommandBuffer, &cmdBufBeginInfo);
}

void gfx_command_end_immediate_recording() {
    vkEndCommandBuffer(g_gfxDevice->vkImmediateCommandBuffer);

    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &g_gfxDevice->vkImmediateCommandBuffer;

    //TODO:GFX replace immediate submit with transfer immediate submit.
    vkQueueSubmit(g_gfxDevice->vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(g_gfxDevice->vkGraphicsQueue);
}