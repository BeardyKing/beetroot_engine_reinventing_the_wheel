#include <gfx/gfx_lit.h>
#include <gfx/gfx_types.h>
#include <gfx/gfx_utils.h>
#include <gfx/gfx_samplers.h>
#include <gfx/gfx_resource_db.h>

#include <shared/assert.h>
#include <shared/log.h>

#include <math/mat4.h>
#include <math/quat.h>
#include <math/utilities.h>

//===runtime sizes=====
#define MAX_FONT_DESCRIPTOR_SET_SIZE 64

struct VulkanFont {
    GfxRenderPass renderPass;

    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;

    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
};
static VulkanFont g_vulkanFont; // questionable static alloc should move to ptr create - destroy

extern struct GfxDevice *g_gfxDevice;

struct GlyphInfo {
    int32_t glyph;              // character range value/index
    int32_t x0, y0, x1, y1;     // coords of glyph in the texture atlas
    int32_t x_off, y_off;       // left & top bearing when rendering
    int32_t advance;            // x advance when rendering
};

#define ATLAS_INFO_VERSION_0 0
struct AtlasInfo {
    uint32_t version;

    uint32_t fontSize;
    uint32_t atlasWidth;
    uint32_t atlasHeight;

    uint32_t glyphCount;
    GlyphInfo *glyphs;
};

AtlasInfo *pipeline_load_atlas_info(const std::string &fileSrc) {
    AtlasInfo *atlasInfo = new AtlasInfo;
    FILE *fileRead = fopen(fileSrc.c_str(), "rb");
    ASSERT_MSG(fileRead != nullptr, "Err: failed to load atlas info at path: %s ", fileSrc.c_str())

    fread(atlasInfo, sizeof(AtlasInfo), 1, fileRead);
    atlasInfo->glyphs = new GlyphInfo[atlasInfo->glyphCount];
    fread(&atlasInfo->glyphs[0], sizeof(GlyphInfo) * atlasInfo->glyphCount, 1, fileRead);

    fclose(fileRead);
    return atlasInfo;
}

#include <map>

void gfx_font_record_render_pass(VkCommandBuffer &cmdBuffer) {
    // get active camera
//    CameraEntity *camEntity = gfx_db_get_camera_entity(0);
//    Camera *camera = gfx_db_get_camera(camEntity->cameraIndex);
//    Transform *camTransform = gfx_db_get_transform(camEntity->transformIndex);
//
//    auto camForward = quat(camTransform->rotation) * WORLD_FORWARD;
//    vec3f lookTarget = camTransform->position + camForward;
//
//    mat4 view = lookAt(camTransform->position, lookTarget, WORLD_UP);
//    mat4 proj = perspective(as_radians(camera->fov), (float) g_gfxDevice->vkExtent.width / (float) g_gfxDevice->vkExtent.height, camera->zNear,
//                            camera->zFar);
//    proj[1][1] *= -1;
//    mat4 viewProj = proj * view;
    static bool debug_once = true;
    static AtlasInfo *atlasInfo = nullptr;
    static std::map<uint32_t, GlyphInfo> glyphMappingTable;
    if (debug_once) {
        debug_once = false;
        atlasInfo = pipeline_load_atlas_info("../res/fonts/JetBrainsMono/JetBrainsMono-Regular.desc");
        for (int i = 0; i < atlasInfo->glyphCount; ++i) {
//            atlasInfo->glyphs[i].glyph;
            log_info("glyph %c : %u\n", atlasInfo->glyphs[i].glyph, atlasInfo->glyphs[i].glyph);
        }
    }
    float width = (float) atlasInfo->atlasWidth;
    float height = (float) atlasInfo->atlasHeight;
    float scaleX = ((float) atlasInfo->glyphs[2].x0 / (float) atlasInfo->glyphs[2].x1) / (width / 2);
    float scaleY = ((float) atlasInfo->glyphs[2].y0 / (float) atlasInfo->glyphs[2].y1) / (height / 2);
    float offsetX = ((float) atlasInfo->glyphs[2].x_off) / (width / 2);
    float offsetY = ((float) atlasInfo->glyphs[2].y_off) / (height / 2);

    log_info("%f, %f, %f, %f, %f, %f \n",
             width, height, scaleX,
             scaleY, offsetX, offsetY);

    mat4 proj = ortho(0.0f, (float) g_gfxDevice->vkExtent.width, 0.0f, (float) g_gfxDevice->vkExtent.height);
    // Record geometry pass
    const uint32_t clearValueCount = 2;
    VkClearValue clearValues[clearValueCount]{};
    clearValues[0].color = {};
    clearValues[1].depthStencil.depth = 1.0f;

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = g_vulkanFont.renderPass.vkRenderPass;
    renderPassBeginInfo.framebuffer = g_vulkanFont.renderPass.vkFramebuffer[g_gfxDevice->swapchainImageIndex];
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent = g_gfxDevice->vkExtent;
    renderPassBeginInfo.clearValueCount = clearValueCount;
    renderPassBeginInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    {
        const uint32_t litEntityCount = gfx_db_get_lit_entity_count();
        for (uint32_t i = 0; i < litEntityCount; ++i) {
            const FontEntity *entity = gfx_db_get_font_entity(i);
            const FontMaterial *material = gfx_db_get_font_material(entity->materialIndex);
            const VkDescriptorSet *descriptorSet = gfx_db_get_descriptor_set(material->descriptorSetIndex);
            const UiTransform *transform = gfx_db_get_ui_transform(entity->uiTransformIndex);
            const GfxMesh *mesh = gfx_db_get_mesh(entity->meshIndex);

            vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_vulkanFont.pipeline);
            vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_vulkanFont.pipelineLayout, 0, 1, descriptorSet, 0, nullptr);

            const vec3f pos = {transform->position, 0};
            const vec3f rot = {transform->rotation, 0};
            vec3f scl = {transform->size.x / (float) g_gfxDevice->vkExtent.width, (transform->size.y / (float) g_gfxDevice->vkExtent.height) * -1, 1};

            const mat4 model = translate(mat4(1.0f), pos) * toMat4(quat(rot)) * scale(mat4(1.0f), scl);
            const vec2f uvOffset = {0.5 - offsetX, 0.5 - offsetY};

            const vec2f uvScale = {scaleX, scaleY};
            FontUniformBufferObject ubo = {};
            ubo.mvp = model;
            ubo.uvOffset = uvOffset;
            ubo.uvScale = uvScale;
            vkCmdPushConstants(cmdBuffer, g_vulkanFont.pipelineLayout, VK_SHADER_STAGE_ALL, 0, sizeof(FontUniformBufferObject), &ubo);

            const VkBuffer vertexBuffers[] = {mesh->vertexBuffer};
            const VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(cmdBuffer, mesh->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(cmdBuffer, mesh->indexCount, 1, 0, 0, 0);
        }
    }
    vkCmdEndRenderPass(cmdBuffer);
}

void gfx_create_font_renderpass(const VkFormat &selectedSurfaceFormat, const VkFormat &selectedDepthFormat) {
    if (g_vulkanFont.renderPass.vkRenderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(g_gfxDevice->vkDevice, g_vulkanFont.renderPass.vkRenderPass, nullptr);
        g_vulkanFont.renderPass.vkRenderPass = VK_NULL_HANDLE;
    }

    VkAttachmentDescription attachments[2];
    memset(attachments, 0, sizeof(attachments));

    attachments[0].format = selectedSurfaceFormat;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
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
    vkCreateRenderPass(g_gfxDevice->vkDevice, &renderPassInfo, nullptr, &g_vulkanFont.renderPass.vkRenderPass);
}

void gfx_create_font_framebuffer() {
    g_vulkanFont.renderPass.frameBufferCount = g_gfxDevice->swapchainImageViewCount;
    g_vulkanFont.renderPass.vkFramebuffer = new VkFramebuffer[g_vulkanFont.renderPass.frameBufferCount];

    for (size_t i = 0; i < g_vulkanFont.renderPass.frameBufferCount; ++i) {
        const uint32_t attachmentsCount = 2;
        VkImageView attachments[attachmentsCount] = {g_gfxDevice->vkSwapchainImageViews[i], g_gfxDevice->depthImageView};

        VkFramebufferCreateInfo framebufferInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        framebufferInfo.renderPass = g_vulkanFont.renderPass.vkRenderPass;
        framebufferInfo.attachmentCount = attachmentsCount;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = g_gfxDevice->vkExtent.width;
        framebufferInfo.height = g_gfxDevice->vkExtent.height;
        framebufferInfo.layers = 1;
        vkCreateFramebuffer(g_gfxDevice->vkDevice, &framebufferInfo, nullptr, &g_vulkanFont.renderPass.vkFramebuffer[i]);
    }
}

void gfx_create_font_descriptors() {
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    descriptorSetLayoutInfo.bindingCount = 1;
    descriptorSetLayoutInfo.pBindings = &samplerLayoutBinding;
    VkResult descriptorLayoutRes = vkCreateDescriptorSetLayout(g_gfxDevice->vkDevice, &descriptorSetLayoutInfo, nullptr,
                                                               &g_vulkanFont.descriptorSetLayout);
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
    descriptorPoolInfo.maxSets = MAX_FONT_DESCRIPTOR_SET_SIZE;
    VkResult descriptorPoolRes = vkCreateDescriptorPool(g_gfxDevice->vkDevice, &descriptorPoolInfo, nullptr,
                                                        &g_vulkanFont.descriptorPool);
    ASSERT_MSG(descriptorPoolRes == VK_SUCCESS, "Err: failed to create descriptor pool")
}

void gfx_create_font_pipeline() {
    {
        char *vertShaderCode = nullptr;
        size_t vertShaderCodeSize{};
        gfx_load_shader_binary("../res/shaders/font/font.vert.spv", &vertShaderCode, vertShaderCodeSize);
        ASSERT_MSG(vertShaderCode != nullptr, "Err: failed to load vert shader");
        ASSERT_MSG(vertShaderCodeSize != 0, "Err: failed to load vert shader");

        VkShaderModuleCreateInfo shaderModuleInfo = {
                VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO
        };
        shaderModuleInfo.codeSize = vertShaderCodeSize;
        shaderModuleInfo.pCode = (const uint32_t *) vertShaderCode;
        VkShaderModule vertShader = VK_NULL_HANDLE;
        vkCreateShaderModule(g_gfxDevice->vkDevice, &shaderModuleInfo, nullptr, &vertShader);

        char *fragShaderCode = nullptr;
        size_t fragShaderCodeSize{};
        gfx_load_shader_binary("../res/shaders/font/font.frag.spv", &fragShaderCode, fragShaderCodeSize);
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
                VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO
        };
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
                VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO
        };
        pipelineColorBlendStateInfo.logicOpEnable = VK_FALSE;
        pipelineColorBlendStateInfo.logicOp = VK_LOGIC_OP_COPY;
        pipelineColorBlendStateInfo.attachmentCount = 1;
        pipelineColorBlendStateInfo.pAttachments = &pipelineColorBlendAttachmentState;

        VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO
        };
        depthStencilStateInfo.depthTestEnable = VK_TRUE;
        depthStencilStateInfo.depthWriteEnable = VK_TRUE;
        depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilStateInfo.stencilTestEnable = VK_FALSE;

        VkGraphicsPipelineCreateInfo pipelineInfo = {
                VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO
        };
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
        pipelineInfo.layout = g_vulkanFont.pipelineLayout;
        pipelineInfo.renderPass = g_vulkanFont.renderPass.vkRenderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        const auto pipelineRes = vkCreateGraphicsPipelines(
                g_gfxDevice->vkDevice,
                VK_NULL_HANDLE,
                1,
                &pipelineInfo,
                nullptr,
                &g_vulkanFont.pipeline
        );
        ASSERT_MSG(pipelineRes == VK_SUCCESS, "Err: failed to create fallback graphics pipeline");

        vkDestroyShaderModule(g_gfxDevice->vkDevice, fragShader, nullptr);
        vkDestroyShaderModule(g_gfxDevice->vkDevice, vertShader, nullptr);

        delete[] vertShaderCode;
        delete[] fragShaderCode;
    }
}

void gfx_destroy_font() {
    {
        for (uint32_t i = 0; i < g_vulkanFont.renderPass.frameBufferCount; ++i) {
            vkDestroyFramebuffer(g_gfxDevice->vkDevice, g_vulkanFont.renderPass.vkFramebuffer[i], nullptr);
        }
        g_vulkanFont.renderPass.frameBufferCount = 0;
        delete[] g_vulkanFont.renderPass.vkFramebuffer;
        g_vulkanFont.renderPass.vkFramebuffer = nullptr;
    }
    {
        vkDestroyPipeline(g_gfxDevice->vkDevice, g_vulkanFont.pipeline, nullptr);
        g_vulkanFont.pipeline = VK_NULL_HANDLE;
    }
    {
        vkDestroyRenderPass(g_gfxDevice->vkDevice, g_vulkanFont.renderPass.vkRenderPass, nullptr);
        g_vulkanFont.renderPass.vkRenderPass = VK_NULL_HANDLE;
    }
    {
        vkDestroyPipelineLayout(g_gfxDevice->vkDevice, g_vulkanFont.pipelineLayout, nullptr);
        g_vulkanFont.pipelineLayout = VK_NULL_HANDLE;
    }
}

void gfx_create_font_pipeline_layout() {
    VkPushConstantRange pushConstantRanges[1]{};
    pushConstantRanges[0].offset = 0;
    pushConstantRanges[0].size = sizeof(FontUniformBufferObject);
    pushConstantRanges[0].stageFlags = VK_SHADER_STAGE_ALL;

    const uint32_t descriptorSetLayoutsCount = 1;
    VkDescriptorSetLayout descriptorSetLayouts[descriptorSetLayoutsCount] = {g_vulkanFont.descriptorSetLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = descriptorSetLayoutsCount;
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges;
    vkCreatePipelineLayout(g_gfxDevice->vkDevice, &pipelineLayoutInfo, nullptr, &g_vulkanFont.pipelineLayout);
}


void gfx_font_update_material_descriptor(VkDescriptorSet &outDescriptorSet, const GfxTexture &albedoTexture) {
// TODO:    This code is needed to update the material descriptors whenever there is a change resource
//          i.e texture/value change. it is likely in release state that most resources would be static
//          but this is not the case when we are using the editor as we update resource values quite often.
//          any all cases we would want to defer updating the resource until the end of the fame so that
//          we don't cause any hitching when updating the descriptors :)

    VkDescriptorSetLayout descriptorSetLayouts[] = {g_vulkanFont.descriptorSetLayout};
    VkDescriptorSetAllocateInfo descriptorSetInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    descriptorSetInfo.descriptorPool = g_vulkanFont.descriptorPool;
    descriptorSetInfo.descriptorSetCount = 1;
    descriptorSetInfo.pSetLayouts = descriptorSetLayouts;
    VkResult allocateDescriptorRes =
            vkAllocateDescriptorSets(g_gfxDevice->vkDevice, &descriptorSetInfo, &outDescriptorSet);
    ASSERT_MSG(allocateDescriptorRes == VK_SUCCESS, "Err: failed to allocate descriptor set");

    VkDescriptorImageInfo descriptorImageInfo = {};
    descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    descriptorImageInfo.imageView = albedoTexture.imageView;
    descriptorImageInfo.sampler = gfx_samplers()->samplers[(TextureSamplerType) albedoTexture.imageSamplerType];

    VkWriteDescriptorSet writeDescriptorSet = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    writeDescriptorSet.dstSet = outDescriptorSet;
    writeDescriptorSet.dstBinding = 1;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.pImageInfo = &descriptorImageInfo;

    vkUpdateDescriptorSets(g_gfxDevice->vkDevice, 1, &writeDescriptorSet, 0, nullptr);
}

void gfx_cleanup_font_descriptors() {
    vkDestroyDescriptorPool(g_gfxDevice->vkDevice, g_vulkanFont.descriptorPool, nullptr);
    g_vulkanFont.descriptorPool = VK_NULL_HANDLE;

    vkDestroyDescriptorSetLayout(g_gfxDevice->vkDevice, g_vulkanFont.descriptorSetLayout, nullptr);
    g_vulkanFont.descriptorSetLayout = VK_NULL_HANDLE;
}