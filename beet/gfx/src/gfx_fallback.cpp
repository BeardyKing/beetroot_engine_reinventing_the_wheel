#include <gfx/gfx_fallback.h>
#include <gfx/gfx_types.h>
#include <gfx/gfx_utils.h>
#include <gfx/gfx_samplers.h>
#include <gfx/gfx_command.h>
#include <gfx/gfx_resource_db.h>

#include <shared/assert.h>
#include <shared/log.h>

#include <math/mat4.h>
#include <math/quat.h>
#include <math/utilities.h>

struct VulkanFallbacks {
    GfxRenderPass renderPass;

    VkBuffer vertexBuffer;
    VmaAllocation vertexAllocation;

    VkBuffer indexBuffer;
    VmaAllocation indexAllocation;
    uint32_t vertexCount;
    uint32_t indexCount;

    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;

    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
};
VulkanFallbacks g_vulkanFallbacks;

extern struct GfxDevice *g_gfxDevice;

void gfx_fallback_record_render_pass(VkCommandBuffer &cmdBuffer) {
    // get active camera
    CameraEntity *camEntity = gfx_db_get_camera_entity(0);
    Camera *camera = gfx_db_get_camera(camEntity->cameraIndex);
    Transform *camTransform = gfx_db_get_transform(camEntity->transformIndex);

    auto camForward = quat(camTransform->rotation) * WORLD_FORWARD;
    vec3f lookTarget = camTransform->position + camForward;

    mat4 view = lookAt(camTransform->position, lookTarget, WORLD_UP);
    mat4 proj = perspective(as_radians(camera->fov), (float) g_gfxDevice->vkExtent.width / (float) g_gfxDevice->vkExtent.height, camera->zNear,
                            camera->zFar);
    proj[1][1] *= -1;
    mat4 viewProj = proj * view;

    // Record geometry pass
    const uint32_t clearValueCount = 2;
    VkClearValue clearValues[clearValueCount]{};
    clearValues[0].color = {{0.5f, 0.092, 0.167f, 1.0f}};
    clearValues[1].depthStencil.depth = 1.0f;

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = g_vulkanFallbacks.renderPass.vkRenderPass;
    renderPassBeginInfo.framebuffer = g_vulkanFallbacks.renderPass.vkFramebuffer[g_gfxDevice->swapchainImageIndex];
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent = g_gfxDevice->vkExtent;
    renderPassBeginInfo.clearValueCount = clearValueCount;
    renderPassBeginInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    {
        LitEntity *entity = gfx_db_get_lit_entity(0);
        LitMaterial *material = gfx_db_get_lit_material(entity->materialIndex);
        VkDescriptorSet *descriptorSet = gfx_db_get_descriptor_set(material->descriptorSetIndex);
        Transform *transform = gfx_db_get_transform(entity->transformIndex);

        vkCmdBindPipeline(
                cmdBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                g_vulkanFallbacks.pipeline
        );

        mat4 model = translate(mat4(1.0f), transform->position) * toMat4(quat(transform->rotation)) * scale(mat4(1.0f), transform->scale);

        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_vulkanFallbacks.pipelineLayout, 0, 1,
                                descriptorSet, 0, nullptr);

        UniformBufferObject ubo = {};
        ubo.mvp = viewProj * model;

        vkCmdPushConstants(cmdBuffer,
                           g_vulkanFallbacks.pipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT,
                           0,
                           sizeof(UniformBufferObject),
                           &ubo
        );

        VkBuffer vertexBuffers[] = {g_vulkanFallbacks.vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(cmdBuffer, g_vulkanFallbacks.indexBuffer, 0, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(cmdBuffer, g_vulkanFallbacks.indexCount, 1, 0, 0, 0);
    }
    vkCmdEndRenderPass(cmdBuffer);
}

void gfx_create_fallback_renderpass(const VkFormat &selectedSurfaceFormat, const VkFormat &selectedDepthFormat) {
    if (g_vulkanFallbacks.renderPass.vkRenderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(g_gfxDevice->vkDevice, g_vulkanFallbacks.renderPass.vkRenderPass, nullptr);
        g_vulkanFallbacks.renderPass.vkRenderPass = VK_NULL_HANDLE;
    }

    VkAttachmentDescription attachments[2];
    memset(attachments, 0, sizeof(attachments));

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
    vkCreateRenderPass(g_gfxDevice->vkDevice, &renderPassInfo, nullptr, &g_vulkanFallbacks.renderPass.vkRenderPass);
}

void gfx_create_fallback_framebuffer() {
    g_vulkanFallbacks.renderPass.frameBufferCount = g_gfxDevice->swapchainImageViewCount;
    g_vulkanFallbacks.renderPass.vkFramebuffer = new VkFramebuffer[g_vulkanFallbacks.renderPass.frameBufferCount];

    for (size_t i = 0; i < g_vulkanFallbacks.renderPass.frameBufferCount; ++i) {
        const uint32_t attachmentsCount = 2;
        VkImageView attachments[attachmentsCount] = {g_gfxDevice->vkSwapchainImageViews[i], g_gfxDevice->depthImageView};

        VkFramebufferCreateInfo framebufferInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        framebufferInfo.renderPass = g_vulkanFallbacks.renderPass.vkRenderPass;
        framebufferInfo.attachmentCount = attachmentsCount;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = g_gfxDevice->vkExtent.width;
        framebufferInfo.height = g_gfxDevice->vkExtent.height;
        framebufferInfo.layers = 1;
        vkCreateFramebuffer(g_gfxDevice->vkDevice, &framebufferInfo, nullptr, &g_vulkanFallbacks.renderPass.vkFramebuffer[i]);
    }
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
    memset(descriptorPoolSizes, 0, sizeof(descriptorPoolSizes));
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


}

void gfx_create_fallback_pipeline() {
    {
        char *vertShaderCode = nullptr;
        size_t vertShaderCodeSize{};
        gfx_load_shader_binary("../res/shaders/fallback/fallback.vert.spv", &vertShaderCode, vertShaderCodeSize);
        ASSERT_MSG(vertShaderCode != nullptr, "Err: failed to load vert shader");
        ASSERT_MSG(vertShaderCodeSize != 0, "Err: failed to load vert shader");

        VkShaderModuleCreateInfo shaderModuleInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        shaderModuleInfo.codeSize = vertShaderCodeSize;
        shaderModuleInfo.pCode = (const uint32_t *) vertShaderCode;
        VkShaderModule vertShader = VK_NULL_HANDLE;
        vkCreateShaderModule(g_gfxDevice->vkDevice, &shaderModuleInfo, nullptr, &vertShader);

        char *fragShaderCode = nullptr;
        size_t fragShaderCodeSize{};
        gfx_load_shader_binary("../res/shaders/fallback/fallback.frag.spv", &fragShaderCode, fragShaderCodeSize);
        ASSERT_MSG(fragShaderCode != nullptr, "Err: failed to load frag shader");
        ASSERT_MSG(fragShaderCodeSize != 0, "Err: failed to load frag shader");

        shaderModuleInfo.codeSize = fragShaderCodeSize;
        shaderModuleInfo.pCode = (const uint32_t *) fragShaderCode;
        VkShaderModule fragShader = VK_NULL_HANDLE;
        vkCreateShaderModule(g_gfxDevice->vkDevice, &shaderModuleInfo, nullptr, &fragShader);

        VkPipelineShaderStageCreateInfo vertPipelineShaderStageInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
        };
        vertPipelineShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertPipelineShaderStageInfo.module = vertShader;
        vertPipelineShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragPipelineShaderStageInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
        };
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
                VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
        };
        pipelineVertexInputStateInfo.vertexBindingDescriptionCount = 1;
        pipelineVertexInputStateInfo.pVertexBindingDescriptions = &bindingDescription;
        pipelineVertexInputStateInfo.vertexAttributeDescriptionCount = _countof(attributeDescriptions);
        pipelineVertexInputStateInfo.pVertexAttributeDescriptions = attributeDescriptions;

        VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO
        };
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
                VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO
        };
        pipelineViewportStateInfo.viewportCount = 1;
        pipelineViewportStateInfo.pViewports = &viewport;
        pipelineViewportStateInfo.scissorCount = 1;
        pipelineViewportStateInfo.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO
        };
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
        pipelineInfo.renderPass = g_vulkanFallbacks.renderPass.vkRenderPass;
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

void gfx_destroy_fallback() {
    {
        for (int i = 0; i < g_vulkanFallbacks.renderPass.frameBufferCount; ++i) {
            vkDestroyFramebuffer(g_gfxDevice->vkDevice, g_vulkanFallbacks.renderPass.vkFramebuffer[i], nullptr);
        }
        g_vulkanFallbacks.renderPass.frameBufferCount = 0;
        delete[] g_vulkanFallbacks.renderPass.vkFramebuffer;
        g_vulkanFallbacks.renderPass.vkFramebuffer = nullptr;
    }
    {
        vkDestroyPipeline(g_gfxDevice->vkDevice, g_vulkanFallbacks.pipeline, nullptr);
        g_vulkanFallbacks.pipeline = VK_NULL_HANDLE;
    }
    {
        vkDestroyRenderPass(g_gfxDevice->vkDevice, g_vulkanFallbacks.renderPass.vkRenderPass, nullptr);
        g_vulkanFallbacks.renderPass.vkRenderPass = VK_NULL_HANDLE;
    }
    {
        vkDestroyPipelineLayout(g_gfxDevice->vkDevice, g_vulkanFallbacks.pipelineLayout, nullptr);
        g_vulkanFallbacks.pipelineLayout = VK_NULL_HANDLE;
    }
}

void gfx_create_fallback_pipeline_layout() {
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


void gfx_cleanup_fallback_texture(GfxTexture &gfxTexture) {
    vkDestroyImageView(g_gfxDevice->vkDevice, gfxTexture.imageView, nullptr);
    gfxTexture.imageView = VK_NULL_HANDLE;

    vmaDestroyImage(g_gfxDevice->vmaAllocator, gfxTexture.imageTexture, gfxTexture.imageAllocation);
    gfxTexture.imageTexture = VK_NULL_HANDLE;
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
            0, 1, 2, 3, UINT16_MAX,
            4, 5, 6, 7, UINT16_MAX,
            8, 9, 10, 11, UINT16_MAX,
            12, 13, 14, 15, UINT16_MAX,
            16, 17, 18, 19, UINT16_MAX,
            20, 21, 22, 23, UINT16_MAX,
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

    gfx_command_begin_immediate_recording();
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
    gfx_command_end_immediate_recording();

    vmaDestroyBuffer(g_gfxDevice->vmaAllocator, stagingIndexBuffer, stagingIndexBufferAlloc);
    vmaDestroyBuffer(g_gfxDevice->vmaAllocator, stagingVertexBuffer, stagingVertexBufferAlloc);
}

void gfx_create_fallback_texture(GfxTexture &outTexture) {
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
            &outTexture.imageTexture,
            &outTexture.imageAllocation,
            nullptr
    );
    ASSERT_MSG(imageRes == VK_SUCCESS, "Err: failed to allocate image");

    gfx_command_begin_immediate_recording();
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
        imgMemBarrier.image = outTexture.imageTexture;
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
                outTexture.imageTexture,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &region
        );

        imgMemBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imgMemBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imgMemBarrier.image = outTexture.imageTexture;
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
    gfx_command_end_immediate_recording();

    vmaDestroyBuffer(g_gfxDevice->vmaAllocator, stagingBuf, stagingBufAlloc);

    VkImageViewCreateInfo textureImageViewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    textureImageViewInfo.image = outTexture.imageTexture;
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
            &outTexture.imageView
    );
    ASSERT_MSG(imageViewRes == VK_SUCCESS, "Err: failed to create image view");
}

void gfx_fallback_update_material_descriptor(VkDescriptorSet &outDescriptorSet, const GfxTexture &albedoTexture) {
// TODO:    This code is needed to update the material descriptors whenever there is a change resource
//          i.e texture/value change. it is likely in release state that most resources would be static
//          but this is not the case when we are using the editor as we update resource values quite often.
//          any all cases we would want to defer updating the resource until the end of the fame so that
//          we don't cause any hitching when updating the descriptors :)

    VkDescriptorSetLayout descriptorSetLayouts[] = {g_vulkanFallbacks.descriptorSetLayout};
    VkDescriptorSetAllocateInfo descriptorSetInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    descriptorSetInfo.descriptorPool = g_vulkanFallbacks.descriptorPool;
    descriptorSetInfo.descriptorSetCount = 1;
    descriptorSetInfo.pSetLayouts = descriptorSetLayouts;
    VkResult allocateDescriptorRes =
            vkAllocateDescriptorSets(g_gfxDevice->vkDevice, &descriptorSetInfo, &outDescriptorSet);
    ASSERT_MSG(allocateDescriptorRes == VK_SUCCESS, "Err: failed to allocate descriptor set");

    VkDescriptorImageInfo descriptorImageInfo = {};
    descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    descriptorImageInfo.imageView = albedoTexture.imageView;
    descriptorImageInfo.sampler = gfx_samplers()->pointSampler;

    VkWriteDescriptorSet writeDescriptorSet = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    writeDescriptorSet.dstSet = outDescriptorSet;
    writeDescriptorSet.dstBinding = 1;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.pImageInfo = &descriptorImageInfo;

    vkUpdateDescriptorSets(g_gfxDevice->vkDevice, 1, &writeDescriptorSet, 0, nullptr);
}

void gfx_cleanup_fallback_mesh() {
    vmaDestroyBuffer(g_gfxDevice->vmaAllocator,
                     g_vulkanFallbacks.indexBuffer,
                     g_vulkanFallbacks.indexAllocation
    );
    g_vulkanFallbacks.indexBuffer = VK_NULL_HANDLE;

    vmaDestroyBuffer(g_gfxDevice->vmaAllocator,
                     g_vulkanFallbacks.vertexBuffer,
                     g_vulkanFallbacks.vertexAllocation
    );
    g_vulkanFallbacks.vertexBuffer = VK_NULL_HANDLE;
}


void gfx_cleanup_fallback_descriptors() {

    vkDestroyDescriptorPool(g_gfxDevice->vkDevice, g_vulkanFallbacks.descriptorPool, nullptr);
    g_vulkanFallbacks.descriptorPool = VK_NULL_HANDLE;

    vkDestroyDescriptorSetLayout(g_gfxDevice->vkDevice, g_vulkanFallbacks.descriptorSetLayout, nullptr);
    g_vulkanFallbacks.descriptorSetLayout = VK_NULL_HANDLE;
}