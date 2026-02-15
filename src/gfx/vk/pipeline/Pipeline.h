#pragma once

#include <vulkan/vulkan.h>
#include <span>
#include "gfx/vk/gpu_resources/Shader.h"

class Pipeline {
   public:
    enum Type {
        Graphics = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    };

    virtual ~Pipeline() = default;

    Pipeline(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
    // virtual ~Pipeline() = default;
    // explicit Pipeline(const char* vertexShaderPath, const char* fragmentShaderPath,
    //                   const VkPipelineVertexInputStateCreateInfo& vertexInputState,
    //                   const VkPipelineInputAssemblyStateCreateInfo& inputAssembly,
    //                   const VkPipelineViewportStateCreateInfo& viewportState,
    //                   const VkPipelineRasterizationStateCreateInfo& rasterizer,
    //                   const VkPipelineMultisampleStateCreateInfo& multisample,
    //                   const VkPipelineColorBlendStateCreateInfo& colorBlendState,
    //                   const VkPipelineDepthStencilStateCreateInfo& depthStencilState, const VkPipelineLayout& layout,
    //                   const VkRenderPass& renderPass);

    [[nodiscard]]
    const VkPipeline& getUnderlying() const;

    [[nodiscard]]
    const VkPipelineLayout& getLayout() const;

    // void bind(const VkCommandBuffer& commandBuffer) const;

    [[nodiscard]]
    virtual VkPipelineVertexInputStateCreateInfo getVertexInputState() const;

    [[nodiscard]]
    virtual VkPipelineViewportStateCreateInfo getViewportState(const VkExtent2D& extent) const;

    [[nodiscard]]
    virtual VkPipelineInputAssemblyStateCreateInfo getInputAssembly() const;

    [[nodiscard]]
    virtual VkPipelineRasterizationStateCreateInfo getRasterizer() const;

    [[nodiscard]]
    virtual VkPipelineMultisampleStateCreateInfo getMultisampler() const;

    [[nodiscard]]
    virtual VkPipelineColorBlendStateCreateInfo getColorBlending() const;

    [[nodiscard]]
    virtual VkPipelineDepthStencilStateCreateInfo getDepthStencil() const;

    [[nodiscard]]
    virtual void createLayout(const std::span<VkDescriptorSetLayout>& setLayouts);

    void destroy() const;

   protected:
    VkPipelineLayout m_layout = VK_NULL_HANDLE;
    VkPipeline m_underlying = VK_NULL_HANDLE;

    std::unique_ptr<Shader> m_vertexShader;
    std::unique_ptr<Shader> m_fragmentShader;
};
