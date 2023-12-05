#include <gfx/gfx_texture.h>
#include <gfx/gfx_command.h>
#include <gfx/gfx_samplers.h>

#include <shared/texture_formats.h>
#include <shared/dds_loader.h>

extern struct GfxDevice *g_gfxDevice;

VkFormat beet_image_format_to_vk(TextureFormat textureFormat) {
    switch (textureFormat) {
        case TextureFormat::RGBA8:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::RGBA16:
            return VK_FORMAT_R16G16B16A16_UNORM;

        case TextureFormat::BC1RGBA:
            return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case TextureFormat::BC2:
            return VK_FORMAT_BC2_UNORM_BLOCK;
        case TextureFormat::BC3:
            return VK_FORMAT_BC3_UNORM_BLOCK;
        case TextureFormat::BC4:
            return VK_FORMAT_BC4_UNORM_BLOCK;
        case TextureFormat::BC5:
            return VK_FORMAT_BC5_UNORM_BLOCK;
        case TextureFormat::BC6H:
            return VK_FORMAT_BC6H_UFLOAT_BLOCK;
        case TextureFormat::BC7:
            return VK_FORMAT_BC7_UNORM_BLOCK;
        default: SANITY_CHECK();
    };
    return VK_FORMAT_UNDEFINED;
}

void gfx_create_texture_immediate(const char* path, GfxTexture &outTexture) {
    ASSERT_MSG(g_gfxDevice->vmaAllocator, "Err: vma allocator hasn't been created yet");
    // TODO: select sampler type during pipeline and pass it through to here
    outTexture.imageSamplerType = TextureSamplerType::Linear;

    RawImage myImage{};
    load_dds_image(path, &myImage);
    auto rawImageData = (unsigned char *) myImage.data;

    const uint32_t sizeX = myImage.width;
    const uint32_t sizeY = myImage.height;
    const uint32 mipMapCount = myImage.mipMapCount;
    const VkDeviceSize imageSize = myImage.dataSize;

    VkBufferCreateInfo stagingBufInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    stagingBufInfo.size = imageSize;
    stagingBufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo stagingBufAllocCreateInfo = {};
    stagingBufAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    stagingBufAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

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
    uint8_t *data;
    vmaMapMemory(g_gfxDevice->vmaAllocator, stagingBufAlloc, (void **) &data);
    memcpy(data, rawImageData, imageSize);
    vmaUnmapMemory(g_gfxDevice->vmaAllocator, stagingBufAlloc);

    VkBufferImageCopy *bufferCopyRegions = (VkBufferImageCopy *) malloc(mipMapCount * sizeof(VkBufferImageCopy));
    uint32_t offset = 0;
    for (uint32_t i = 0; i < mipMapCount; i++) {
        // setup a buffer image copy structure for the current mip level
        VkBufferImageCopy bufferCopyRegion = {};
        bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        bufferCopyRegion.imageSubresource.mipLevel = i;
        bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
        bufferCopyRegion.imageSubresource.layerCount = 1;
        bufferCopyRegion.imageExtent.width = myImage.width >> i;
        bufferCopyRegion.imageExtent.height = myImage.height >> i;
        bufferCopyRegion.imageExtent.depth = 1;
        bufferCopyRegion.bufferOffset = offset;
        bufferCopyRegions[i] = bufferCopyRegion;
        offset += myImage.mipDataSizes[i];
    }

    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = sizeX;
    imageInfo.extent.height = sizeY;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipMapCount;
    imageInfo.arrayLayers = 1;
    imageInfo.format = beet_image_format_to_vk(myImage.textureFormat);
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
            &outTexture.imageTexture,
            &outTexture.imageAllocation,
            nullptr
    );
    ASSERT_MSG(imageRes == VK_SUCCESS, "Err: failed to allocate image");

    gfx_command_begin_immediate_recording();

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = mipMapCount;
    subresourceRange.layerCount = 1;

    VkImageMemoryBarrier imageMemoryBarrier{};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.image = outTexture.imageTexture;
    imageMemoryBarrier.subresourceRange = subresourceRange;
    imageMemoryBarrier.srcAccessMask = 0;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    vkCmdPipelineBarrier(
            g_gfxDevice->vkImmediateCommandBuffer,
            VK_PIPELINE_STAGE_HOST_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);

    vkCmdCopyBufferToImage(
            g_gfxDevice->vkImmediateCommandBuffer,
            stagingBuf,
            outTexture.imageTexture,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            static_cast<uint32_t>(mipMapCount),
            &bufferCopyRegions[0]);

    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    vkCmdPipelineBarrier(
            g_gfxDevice->vkImmediateCommandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);

    outTexture.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    gfx_command_end_immediate_recording();

    VkImageViewCreateInfo view{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view.format = beet_image_format_to_vk(myImage.textureFormat);;
    view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view.subresourceRange.baseMipLevel = 0;
    view.subresourceRange.baseArrayLayer = 0;
    view.subresourceRange.layerCount = 1;
    view.subresourceRange.levelCount = mipMapCount;
    view.image = outTexture.imageTexture;
    vkCreateImageView(g_gfxDevice->vkDevice, &view, nullptr, &outTexture.imageView);

    vmaDestroyBuffer(g_gfxDevice->vmaAllocator, stagingBuf, stagingBufAlloc);
    free(myImage.data);
    free(bufferCopyRegions);
    myImage.data = nullptr;
}

void gfx_cleanup_texture(GfxTexture &gfxTexture) {
    vkDestroyImageView(g_gfxDevice->vkDevice, gfxTexture.imageView, nullptr);
    gfxTexture.imageView = VK_NULL_HANDLE;

    vmaDestroyImage(g_gfxDevice->vmaAllocator, gfxTexture.imageTexture, gfxTexture.imageAllocation);
    gfxTexture.imageTexture = VK_NULL_HANDLE;

    //TODO:GFX We don't re-add this as a free slot in the texture pool i.e.
    //we could address this pretty simply in a few ways.
    //create free-list for each pool and next time we try and create a new texture to check if any free list spaces are free
    //we move the last image loaded into the newly free position and fix up and dependency, this will break any cached references.
}
