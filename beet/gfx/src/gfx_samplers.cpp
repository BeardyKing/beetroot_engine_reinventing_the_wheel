#include <gfx/gfx_samplers.h>
#include <gfx/gfx_types.h>

#include <shared/assert.h>


#include <cfloat>

extern struct GfxDevice *g_gfxDevice;

TextureSamplers *g_textureSamplers;

TextureSamplers *gfx_samplers() {
    return g_textureSamplers;
}

void gfx_build_samplers() {
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
        samplerInfo.maxLod = 10;
        VkResult pointResult = vkCreateSampler(g_gfxDevice->vkDevice, &samplerInfo, nullptr,
                                               &g_textureSamplers->linearMipSampler);
        ASSERT_MSG(pointResult == VK_SUCCESS, "Err: failed to create linear sampler");
    }
}

void cleanup_samplers() {
    vkDestroySampler(g_gfxDevice->vkDevice, g_textureSamplers->linearSampler, nullptr);
    g_textureSamplers->linearSampler = VK_NULL_HANDLE;

    vkDestroySampler(g_gfxDevice->vkDevice, g_textureSamplers->linearMipSampler, nullptr);
    g_textureSamplers->linearMipSampler = VK_NULL_HANDLE;

    vkDestroySampler(g_gfxDevice->vkDevice, g_textureSamplers->pointSampler, nullptr);
    g_textureSamplers->pointSampler = VK_NULL_HANDLE;
}

void gfx_create_samplers() {
    g_textureSamplers = new TextureSamplers;
    gfx_build_samplers();
}

void gfx_cleanup_samplers() {
    cleanup_samplers();

    delete g_textureSamplers;
    g_textureSamplers = nullptr;
}