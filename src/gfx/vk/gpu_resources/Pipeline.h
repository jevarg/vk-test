#pragma once

#include <vulkan/vulkan.h>

class Pipeline {
   public:
    explicit Pipeline(const VkPipelineShaderStageCreateInfo shaderStages[],
                      const VkPipelineVertexInputStateCreateInfo& vertexInputState,
                      const VkPipelineInputAssemblyStateCreateInfo& inputAssembly,
                      const VkViewport& viewport,
                      const VkRect2D& scissor,
                      const VkPipelineViewportStateCreateInfo& viewportState,
                      const VkPipelineRasterizationStateCreateInfo& rasterizer,
                      const VkPipelineMultisampleStateCreateInfo& multisample,
                      const VkPipelineColorBlendStateCreateInfo& colorBlendState,
                      const VkPipelineDepthStencilStateCreateInfo& depthStencilState,
                      const VkPipelineLayout& layout,
                      const VkRenderPass& renderPass,
                      const VkPushConstantRange& pushConstants);

    [[nodiscard]]
    const VkPipeline& getUnderlying() const;

   private:
    VkPipeline m_underlying;
};
