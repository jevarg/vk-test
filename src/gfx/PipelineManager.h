#pragma once

#include <vulkan/vulkan_core.h>

#include <span>
#include <unordered_map>

class Pipeline;
enum class PipelineType : uint8_t {
    Simple = 0,
};

class PipelineManager {
    using TypeInfoRef = std::reference_wrapper<const std::type_info>;

    struct Hasher
    {
        std::size_t operator()(const TypeInfoRef code) const
        {
            return code.get().hash_code();
        }
    };

    struct EqualTo
    {
        bool operator()(const TypeInfoRef lhs, const TypeInfoRef rhs) const
        {
            return lhs.get() == rhs.get();
        }
    };

public:
    template <class T> requires std::is_base_of_v<Pipeline, T>
    T& get() {
        return static_cast<T&>(*m_pipelines.at(typeid(T)));
    }

    template <class T> requires std::is_base_of_v<Pipeline, T>
    void create(const VkExtent2D& extent, const std::span<VkDescriptorSetLayout>& setLayouts, VkRenderPass renderPass) {
        m_pipelines.emplace(typeid(T), std::make_unique<T>(extent, setLayouts, renderPass));
    }

private:
    std::unordered_map<TypeInfoRef, std::unique_ptr<Pipeline>, Hasher, EqualTo> m_pipelines;
};
