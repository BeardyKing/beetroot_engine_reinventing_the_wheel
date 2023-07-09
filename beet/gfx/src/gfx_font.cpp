//#include <gfx/gfx_font.h>
//#include <gfx/gfx_interface.h>
//#include <gfx/gfx_types.h>
//
//#include <shared/assert.h>
//
//extern struct GfxDevice *g_gfxDevice;
//
//
//struct VulkanFont {
//    VkImage imageTexture;
//    VmaAllocation imageAllocation;
//    VkImageView imageView;
//
//    VkBuffer vertexBuffer;
//    VmaAllocation vertexAllocation;
//
//    VkBuffer indexBuffer;
//    VmaAllocation indexAllocation;
//    uint32_t vertexCount;
//    uint32_t indexCount;
//
//    VkDescriptorSetLayout descriptorSetLayout;
//    VkDescriptorPool descriptorPool;
//    VkDescriptorSet descriptorSet;
//
//    VkPipelineLayout pipelineLayout;
//    VkRenderPass renderPass;
//    VkPipeline pipeline;
//};
//
//VulkanFont g_vulkanFont;
//
//void gfx_create_font_pipeline() {
//    char *vertShaderCode = nullptr;
//    size_t vertShaderCodeSize{};
//    gfx_load_shader_binary("../res/shaders/fallback/fallback.vert.spv", &vertShaderCode, vertShaderCodeSize);
//    ASSERT_MSG(vertShaderCode != nullptr, "Err: failed to load vert shader");
//    ASSERT_MSG(vertShaderCodeSize != 0, "Err: failed to load vert shader");
//
//    VkShaderModuleCreateInfo shaderModuleInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
//    shaderModuleInfo.codeSize = vertShaderCodeSize;
//    shaderModuleInfo.pCode = (const uint32_t *) vertShaderCode;
//    VkShaderModule vertShader = VK_NULL_HANDLE;
//    vkCreateShaderModule(g_gfxDevice->vkDevice, &shaderModuleInfo, nullptr, &vertShader);
//
//    char *fragShaderCode = nullptr;
//    size_t fragShaderCodeSize{};
//    gfx_load_shader_binary("../res/shaders/fallback/fallback.frag.spv", &fragShaderCode, fragShaderCodeSize);
//    ASSERT_MSG(fragShaderCode != nullptr, "Err: failed to load frag shader");
//    ASSERT_MSG(fragShaderCodeSize != 0, "Err: failed to load frag shader");
//}