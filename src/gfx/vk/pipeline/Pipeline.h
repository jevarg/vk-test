#pragma once

#include <vulkan/vulkan.h>

#include "gfx/vk/gpu_resources/Shader.h"

class Pipeline {
   public:
    enum Type {
        Graphics = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    };

    explicit Pipeline(Type type, const char* vertexShaderPath, const char* fragmentShaderPath,
                      const VkPipelineVertexInputStateCreateInfo& vertexInputState,
                      const VkPipelineInputAssemblyStateCreateInfo& inputAssembly,
                      const VkPipelineViewportStateCreateInfo& viewportState,
                      const VkPipelineRasterizationStateCreateInfo& rasterizer,
                      const VkPipelineMultisampleStateCreateInfo& multisample,
                      const VkPipelineColorBlendStateCreateInfo& colorBlendState,
                      const VkPipelineDepthStencilStateCreateInfo& depthStencilState, const VkPipelineLayout& layout,
                      const VkRenderPass& renderPass);

    [[nodiscard]]
    const VkPipeline& getUnderlying() const;

    void bind(const VkCommandBuffer& commandBuffer) const;

    void destroy() const;

   private:
    VkPipeline m_underlying = VK_NULL_HANDLE;

    Shader m_vertexShader;
    Shader m_fragmentShader;
};
