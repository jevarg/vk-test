#include "SimplePipeline.h"

#include "gfx/vk/types/Vertex.h"
#include "gfx/vk/vkutil.h"

SimplePipeline::SimplePipeline(const VkExtent2D& extent, const std::span<VkDescriptorSetLayout>& setLayouts,
                               VkRenderPass renderPass)
    : Pipeline("./shaders/tri.vert", "./shaders/tri.frag") {
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = m_vertexShader->getModule();
    vertShaderStageInfo.pName = m_vertexShader->getEntryPoint();

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = m_fragmentShader->getModule();
    fragShaderStageInfo.pName = m_fragmentShader->getEntryPoint();

    const VkPipelineShaderStageCreateInfo shaderStages[] = {
        vertShaderStageInfo,
        fragShaderStageInfo,
    };

    auto vertexInputState = Pipeline::getVertexInputState();
    auto inputAssembly = Pipeline::getInputAssembly();
    auto viewportState = Pipeline::getViewportState(extent);
    auto rasterizer = Pipeline::getRasterizer();
    auto multisample = Pipeline::getMultisampler();
    auto colorBlendState = Pipeline::getColorBlending();
    auto depthStencilState = Pipeline::getDepthStencil();

    Pipeline::createLayout(setLayouts);

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputState;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisample;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlendState;
    pipelineInfo.pDepthStencilState = &depthStencilState;
    // TODO: Later maybe
    // pipelineInfo.pDynamicState = &dynamicStateInfo;
    pipelineInfo.layout = m_layout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    // TODO: Use pipeline cache?
    VK_CHECK("failed to create graphics pipeline!",
             vkCreateGraphicsPipelines(VulkanContext::get().getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                       &m_underlying));
}
