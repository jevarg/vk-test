#pragma once

#include <glm/glm.hpp>
#include <memory>

#include "common/Transform.h"
#include "objects/Node.h"
#include "vk/gpu_resources/Buffer.h"

class Camera {
public:
    explicit Camera(float aspectRatio);

    void destroy() const;
    void update(float delta);

    [[nodiscard]]
    glm::mat4 getView() const;

    [[nodiscard]]
    const glm::mat4& getProjection() const;

    [[nodiscard]]
    const Buffer& getUniform() const;

    [[nodiscard]]
    VkDescriptorSetLayout getDescriptorSetLayout() const;

    [[nodiscard]]
    VkDescriptorSet getDescriptorSet() const;

    void setPosition(const glm::vec3& v);

private:
    void m_createDescriptorSet();

    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;

    std::unique_ptr<Buffer> m_uniformBuffer;
    Node m_node;
    glm::mat4 m_projection{};

    float m_speed = 0.1f;
    float m_sensitivity = 1.0f;
};

