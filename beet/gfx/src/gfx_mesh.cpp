#include <gfx/gfx_mesh.h>
#include <gfx/gfx_command.h>

#include <shared/assert.h>

extern struct GfxDevice *g_gfxDevice;

void gfx_create_plane_immediate(GfxMesh &outMesh) {
    const uint32_t vertexCount = 4;
    static Vertex vertices[vertexCount] = {
            //===POS================//===COLOUR===========//===UV===
            // +z
            {{-1.0f, -1.0f, 1.0f},  {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{1.0f,  -1.0f, 1.0f},  {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{-1.0f, 1.0f,  1.0f},  {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{1.0f,  1.0f,  1.0f},  {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    };

    const uint32_t indexCount = 5;
    static uint32_t indices[indexCount] = {
            0, 1, 2, 3, UINT32_MAX,
    };
    RawMesh rawMesh = {};
    rawMesh.vertexSize = vertexCount;
    rawMesh.vertexData = vertices;
    rawMesh.indexSize = indexCount;
    rawMesh.indexData = indices;
    gfx_create_mesh_immediate(rawMesh, outMesh);
}

void gfx_create_cube_immediate(GfxMesh &outMesh) {
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
    static uint32_t indices[indexCount] = {
            0, 1, 2, 3, UINT32_MAX,
            4, 5, 6, 7, UINT32_MAX,
            8, 9, 10, 11, UINT32_MAX,
            12, 13, 14, 15, UINT32_MAX,
            16, 17, 18, 19, UINT32_MAX,
            20, 21, 22, 23, UINT32_MAX,
    };
    RawMesh rawMesh = {};
    rawMesh.vertexSize = vertexCount;
    rawMesh.vertexData = vertices;
    rawMesh.indexSize = indexCount;
    rawMesh.indexData = indices;
    gfx_create_mesh_immediate(rawMesh, outMesh);
}

void gfx_create_mesh_immediate(const RawMesh& rawMesh, GfxMesh &outMesh) {
    ASSERT_MSG(g_gfxDevice->vmaAllocator, "Err: vma allocator hasn't been created yet");

    const uint32_t vertexCount = rawMesh.vertexSize;
    const uint32_t indexCount = rawMesh.indexSize;
    Vertex* vertices = rawMesh.vertexData;
    uint32_t* indices = rawMesh.indexData;


    size_t vertexBufferSize = sizeof(Vertex) * vertexCount;
    size_t indexBufferSize = sizeof(uint32_t ) * indexCount;
    outMesh.indexCount = indexCount;

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
            &outMesh.vertexBuffer,
            &outMesh.vertexAllocation,
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
            &outMesh.indexBuffer,
            &outMesh.indexAllocation,
            nullptr
    );
    ASSERT_MSG(indexBufferRes == VK_SUCCESS, "Err: failed to create index staging buffer");

    // Copy buffers

    gfx_command_begin_immediate_recording();
    {
        VkBufferCopy vbCopyRegion = {};
        vbCopyRegion.srcOffset = 0;
        vbCopyRegion.dstOffset = 0;
        vbCopyRegion.size = vbInfo.size;
        vkCmdCopyBuffer(
                g_gfxDevice->vkImmediateCommandBuffer,
                stagingVertexBuffer,
                outMesh.vertexBuffer,
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
                outMesh.indexBuffer,
                1,
                &ibCopyRegion
        );
    }
    gfx_command_end_immediate_recording();

    vmaDestroyBuffer(g_gfxDevice->vmaAllocator, stagingIndexBuffer, stagingIndexBufferAlloc);
    vmaDestroyBuffer(g_gfxDevice->vmaAllocator, stagingVertexBuffer, stagingVertexBufferAlloc);
}

void gfx_cleanup_mesh(GfxMesh &mesh) {
    vmaDestroyBuffer(g_gfxDevice->vmaAllocator, mesh.indexBuffer, mesh.indexAllocation);
    mesh.indexBuffer = VK_NULL_HANDLE;

    vmaDestroyBuffer(g_gfxDevice->vmaAllocator, mesh.vertexBuffer, mesh.vertexAllocation);
    mesh.vertexBuffer = VK_NULL_HANDLE;
}