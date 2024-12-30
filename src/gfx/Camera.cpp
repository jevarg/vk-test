#include "Camera.h"

#include <glm/ext/matrix_clip_space.hpp>

#include "input/Keyboard.h"
#include "input/Mouse.h"
#include "vk/types/UniformBufferObject.h"
#include "vk/vkutil.h"

Camera::Camera(const float aspectRatio, const VkDescriptorPool& descriptorPool,
               const VkDescriptorSetLayout& descriptorSetLayout) {
    m_projection = glm::perspective(glm::radians(60.0f), aspectRatio, 0.1f, 10.0f);
    m_projection[1][1] *= -1;  // inverting y because vulkan != gl

    m_uniformBuffer =
        std::make_unique<Buffer>(sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    m_createDescriptorSet(descriptorPool, descriptorSetLayout);
    update(0);
}

void Camera::destroy() const {
    m_uniformBuffer->destroy();
}

glm::mat4 Camera::getView() const {
    const glm::vec3 fwdVec = m_transform.rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    return lookAt(m_transform.position, m_transform.position + fwdVec, glm::vec3(0.0f, 1.0f, 0.0f));
}

const glm::mat4& Camera::getProjection() const {
    return m_projection;
}

const Buffer& Camera::getUniform() const {
    return *m_uniformBuffer;
}

const VkDescriptorSet& Camera::getDescriptorSet() const {
    return m_descriptorSet;
}

void Camera::update(const float delta) {
    if (Keyboard::isKeyPressed(SDL_SCANCODE_W)) {
        localTranslate({ 0.0f, 0.0f, -m_speed });
    }

    if (Keyboard::isKeyPressed(SDL_SCANCODE_S)) {
        localTranslate({ 0.0f, 0.0f, m_speed });
    }

    if (Keyboard::isKeyPressed(SDL_SCANCODE_A)) {
        localTranslate({ -m_speed, 0.0f, 0.0f });
    }

    if (Keyboard::isKeyPressed(SDL_SCANCODE_D)) {
        localTranslate({ m_speed, 0.0f, 0.0f });
    }

    const glm::vec2& mouseDelta = Mouse::getDelta();
    if (mouseDelta.x != 0) {
        rotate(-m_sensitivity * mouseDelta.x, { 0.0f, 1.0f, 0.0f });
    }

    if (mouseDelta.y != 0) {
        rotate(-m_sensitivity * mouseDelta.y, { 1.0f, 0.0f, 0.0f });
    }

    const UniformBufferObject ubo{
        .view = getView(),
        .projection = m_projection,
    };

    // TODO: update only if changed
    m_uniformBuffer->setMemory(&ubo);
}

void Camera::m_createDescriptorSet(const VkDescriptorPool& descriptorPool,
                                   const VkDescriptorSetLayout& descriptorSetLayout) {
    const VulkanContext& vkContext = VulkanContext::get();
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    VK_CHECK("failed to allocate descriptor sets",
             vkAllocateDescriptorSets(vkContext.getDevice(), &allocInfo, &m_descriptorSet));

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = m_uniformBuffer->buffer();
    bufferInfo.range = m_uniformBuffer->getSize();
    bufferInfo.offset = 0;

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(VulkanContext::get().getDevice(), 1, &descriptorWrite, 0, nullptr);
}