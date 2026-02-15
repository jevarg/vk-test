#include "Camera.h"

#include <glm/ext/matrix_clip_space.hpp>

#include "input/Keyboard.h"
#include "input/Mouse.h"
#include "vk/types/UniformBufferObject.h"
#include "vk/vkutil.h"
#include "vk/types/VulkanContext.h"

Camera::Camera(const float aspectRatio) {
    m_projection = glm::perspective(glm::radians(60.0f), aspectRatio, 0.01f, 1000.0f);

    // TODO: change that
    m_projection[1][1] *= -1; // inverting y because vulkan != gl

    m_uniformBuffer =
        std::make_unique<Buffer>(sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    m_createDescriptorSet();
    update(0);
}

void Camera::destroy() const {
    m_uniformBuffer->destroy();
}

glm::mat4 Camera::getView() const {
    const glm::vec3& pos = m_node.getTransform().getPosition();
    const glm::vec3 fwdVec = m_node.getTransform().getRotation() * glm::vec3(0.0f, 0.0f, -1.0f);
    return lookAt(pos, pos + fwdVec, glm::vec3(0.0f, 1.0f, 0.0f));
}

const glm::mat4& Camera::getProjection() const {
    return m_projection;
}

const Buffer& Camera::getUniform() const {
    return *m_uniformBuffer;
}

VkDescriptorSetLayout Camera::getDescriptorSetLayout() const {
    return m_descriptorSetLayout;
}

VkDescriptorSet Camera::getDescriptorSet() const {
    return m_descriptorSet;
}

void Camera::setPosition(const glm::vec3& v) {
    m_node.setPosition(v);
}

void Camera::update(const float delta) {
    float speed = m_speed;
    if (Keyboard::isModPressed(KMOD_LCTRL)) {
        speed *= 0.07;
    }

    if (Keyboard::isKeyPressed(SDL_SCANCODE_W)) {
        m_node.translateLocal({ 0.0f, 0.0f, -speed });
    }

    if (Keyboard::isKeyPressed(SDL_SCANCODE_S)) {
        m_node.translateLocal({ 0.0f, 0.0f, speed });
    }

    if (Keyboard::isKeyPressed(SDL_SCANCODE_A)) {
        m_node.translateLocal({ -speed, 0.0f, 0.0f });
    }

    if (Keyboard::isKeyPressed(SDL_SCANCODE_D)) {
        m_node.translateLocal({ speed, 0.0f, 0.0f });
    }

    if (Keyboard::isKeyPressed(SDL_SCANCODE_SPACE)) {
        m_node.translate({ 0.0f, speed, 0.0f });
    }

    if (Keyboard::isKeyPressed(SDL_SCANCODE_LSHIFT)) {
        m_node.translate({ 0.0f, -speed, 0.0f });
    }

    const glm::vec2& mouseDelta = Mouse::getDelta();
    if (mouseDelta.x != 0) {
        m_node.rotate(-m_sensitivity * mouseDelta.x, { 0.0f, 1.0f, 0.0f });
    }

    if (mouseDelta.y != 0) {
        m_node.rotateLocal(-m_sensitivity * mouseDelta.y, { 1.0f, 0.0f, 0.0f });
    }

    const UniformBufferObject ubo{
        .view = getView(),
        .projection = m_projection,
    };

    // TODO: update only if changed
    m_uniformBuffer->setMemory(&ubo);
}

void Camera::m_createDescriptorSet() {
    constexpr VkDescriptorPoolSize poolSize{
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1; // m_swapChainImages.size(); // TODO: Do that

    VK_CHECK("failed to create camera descriptor pool",
             vkCreateDescriptorPool(VulkanContext::get().getDevice(), &poolInfo, nullptr, &m_descriptorPool));

    VkDescriptorSetLayoutBinding sceneLayoutBinding{};
    sceneLayoutBinding.binding = 0;
    sceneLayoutBinding.descriptorCount = 1;
    sceneLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    sceneLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &sceneLayoutBinding;

    VK_CHECK("failed to create camera descriptor set layout",
             vkCreateDescriptorSetLayout(VulkanContext::get().getDevice(), &layoutInfo, nullptr,
                 &m_descriptorSetLayout));

    const VulkanContext& vkContext = VulkanContext::get();
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_descriptorSetLayout;

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