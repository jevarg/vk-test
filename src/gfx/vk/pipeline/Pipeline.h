#pragma once

#include <vulkan/vulkan.h>
#include <span>
#include "gfx/vk/gpu_resources/Shader.h"

class Pipeline {
   public:
    enum Type {
        Invalid = 0,
        Simple,
        Skybox,
    };

    virtual ~Pipeline() = default;

    Pipeline(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);

    [[nodiscard]]
    Type getType() const;

    [[nodiscard]]
    const VkPipeline& getUnderlying() const;

    [[nodiscard]]
    const VkPipelineLayout& getLayout() const;

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

    virtual void createLayout(const std::span<VkDescriptorSetLayout>& setLayouts);

    void destroy() const;

   protected:
    Type m_type;

    VkPipelineLayout m_layout = VK_NULL_HANDLE;
    VkPipeline m_underlying = VK_NULL_HANDLE;

    std::unique_ptr<Shader> m_vertexShader;
    std::unique_ptr<Shader> m_fragmentShader;
};
