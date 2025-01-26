#pragma once

#include <glm/glm.hpp>

#include "common/Thing.h"
#include "common/Transform.h"
#include "vk/gpu_resources/Buffer.h"

class Camera : public Thing {
   public:
    explicit Camera(float aspectRatio, const VkDescriptorPool& descriptorPool,
                    const VkDescriptorSetLayout& descriptorSetLayout);

    void destroy() const;
    void update(float delta);

    [[nodiscard]]
    glm::mat4 getView() const;

    [[nodiscard]]
    const glm::mat4& getProjection() const;

    [[nodiscard]]
    const Buffer& getUniform() const;

    [[nodiscard]]
    const VkDescriptorSet& getDescriptorSet() const;

   private:
    void m_createDescriptorSet(const VkDescriptorPool& descriptorPool,
                               const VkDescriptorSetLayout& descriptorSetLayout);

    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
    std::unique_ptr<Buffer> m_uniformBuffer;
    glm::mat4 m_projection{};

    float m_speed = 0.1f;
    float m_sensitivity = 1.0f;
};

