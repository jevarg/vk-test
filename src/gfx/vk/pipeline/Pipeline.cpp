#include "Pipeline.h"

#include <fmt/format.h>

#include "gfx/vk/types/VulkanContext.h"
#include "gfx/vk/vkutil.h"

Pipeline::Pipeline(const Type type, const char* vertexShaderPath, const char* fragmentShaderPath,
                   const VkPipelineVertexInputStateCreateInfo& vertexInputState,
                   const VkPipelineInputAssemblyStateCreateInfo& inputAssembly,
                   const VkPipelineViewportStateCreateInfo& viewportState,
                   const VkPipelineRasterizationStateCreateInfo& rasterizer,
                   const VkPipelineMultisampleStateCreateInfo& multisample,
                   const VkPipelineColorBlendStateCreateInfo& colorBlendState,
                   const VkPipelineDepthStencilStateCreateInfo& depthStencilState, const VkPipelineLayout& layout,
                   const VkRenderPass& renderPass)
    : m_vertexShader(vertexShaderPath, Shader::Type::Vertex),
      m_fragmentShader(fragmentShaderPath, Shader::Type::Fragment) {
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = m_vertexShader.getModule();
    vertShaderStageInfo.pName = m_vertexShader.getEntryPoint();

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = m_fragmentShader.getModule();
    fragShaderStageInfo.pName = m_fragmentShader.getEntryPoint();

    VkPipelineShaderStageCreateInfo shaderStages[] = {
        vertShaderStageInfo,
        fragShaderStageInfo,
    };

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = static_cast<VkStructureType>(type);
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
    pipelineInfo.layout = layout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    // TODO: Use pipeline cache?
    VK_CHECK("failed to create graphics pipeline!",
             vkCreateGraphicsPipelines(VulkanContext::get().getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                       &m_underlying));
}

const VkPipeline& Pipeline::getUnderlying() const {
    return m_underlying;
}

void Pipeline::bind(const VkCommandBuffer& commandBuffer) const {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_underlying);
}

void Pipeline::destroy() const {
    m_vertexShader.destroy();
    m_fragmentShader.destroy();
    vkDestroyPipeline(VulkanContext::get().getDevice(), m_underlying, nullptr);
}