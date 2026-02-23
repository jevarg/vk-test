#pragma once

#include <vulkan/vulkan_core.h>

#include <span>
#include <unordered_map>

#include "vk/pipeline/Pipeline.h"

class Pipeline;
enum class PipelineType : uint8_t {
    Simple = 0,
};

class PipelineManager {
public:
    template <class T = Pipeline> requires std::is_base_of_v<Pipeline, T>
    T& get(const Pipeline::Type type) {
        return static_cast<T&>(*m_pipelines.at(type));
    }

    template <class T> requires std::is_base_of_v<Pipeline, T>
    void create(const VkExtent2D& extent, const std::span<VkDescriptorSetLayout>& setLayouts, VkRenderPass renderPass) {
        auto pipeline = std::make_unique<T>(extent, setLayouts, renderPass);
        m_pipelines.emplace(pipeline->getType(), std::move(pipeline));
    }

private:
    std::unordered_map<Pipeline::Type, std::unique_ptr<Pipeline>> m_pipelines;
};
