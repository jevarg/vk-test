#include "MaterialManager.h"

#include <fmt/base.h>
#include <fmt/format.h>

#include "TextureManager.h"
#include "objects/BasicMaterial.h"
#include "vk/vkutil.h"

MaterialManager::MaterialManager(TextureManager& textureManager, PipelineManager& pipelineManager,
                                 const uint32_t maxSize)
    : m_textureManager(textureManager), m_pipelineManager(pipelineManager) {
    const VkDescriptorPoolSize poolSize{
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = maxSize
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = maxSize;

    VK_CHECK("failed to create descriptor pool",
             vkCreateDescriptorPool(VulkanContext::get().getDevice(), &poolInfo, nullptr, &m_descriptorPool));

    VkDescriptorSetLayoutBinding textureLayoutBinding{};
    textureLayoutBinding.binding = 0;
    textureLayoutBinding.descriptorCount = 1;
    textureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &textureLayoutBinding;

    VK_CHECK("failed to create texture descriptor set layout",
             vkCreateDescriptorSetLayout(VulkanContext::get().getDevice(), &layoutInfo, nullptr,
                 &m_descriptorSetLayout));

    m_defaultMaterial = load({ .name = "default-material", .baseColorTexture = "./assets/souley.png" });
}

Handle<BasicMaterial> MaterialManager::load(const MaterialData& data) {
    Handle<Texture> textureHandle = m_textureManager.load(data.baseColorTexture, m_descriptorPool, m_descriptorSetLayout);

    auto material = std::make_shared<BasicMaterial>(data.name, Pipeline::Graphics, textureHandle);

    const auto [it, success] = m_materials.emplace(Handle<BasicMaterial>(), material);
    if (!success) {
        throw std::runtime_error(fmt::format("Unable to load material '{}'", data.name));
    }

    return it->first;
}

std::shared_ptr<BasicMaterial> MaterialManager::get(const Handle<BasicMaterial>& handle) {
    const auto it = m_materials.find(handle);
    if (it != m_materials.end()) {
        fmt::println("MaterialManager::get({}): {}", handle.id, fmt::ptr(it->second.get()));
        return it->second;
    }

    return m_materials.at(m_defaultMaterial);
}

VkDescriptorSetLayout MaterialManager::getDescriptorSetLayout() const {
    return m_descriptorSetLayout;
}