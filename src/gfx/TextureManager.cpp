#include "TextureManager.h"

#include "vk/gpu_resources/Texture.h"
#include "vk/types/VulkanContext.h"
#include "vk/vkutil.h"

TextureManager::TextureManager(VkDescriptorPool descriptorPool): m_descriptorPool(descriptorPool) {
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
}

// TextureManager::~TextureManager() {
//     for (const auto pair : m_textures) {
//         pair.second->destroy();
//     }
// }

TextureHandle TextureManager::loadTexture(const std::string& filePath) {
    const auto it = m_textureCache.find(filePath);
    if (it != m_textureCache.end()) {
        return it->second;
    }

    TextureHandle handle;
    m_textures.emplace(handle, std::make_shared<Texture>(filePath, m_descriptorPool, m_descriptorSetLayout));

    fmt::println("[{}] Loaded {}", handle.handle, filePath);

    return handle;
}

TextureHandle TextureManager::loadCubeMap(const std::span<const std::string>& filePaths) {
    fmt::println("Loading Cube texture... (Warning: not cached)");

    TextureHandle handle;
    m_textures.emplace(handle, std::make_shared<Texture>(filePaths, m_descriptorPool, m_descriptorSetLayout));

    for (const auto& path : filePaths) {
        fmt::println("[{}] Loaded {}", handle.handle, path);
    }

    return handle;
}

TextureHandle TextureManager::loadCubeMap(const std::initializer_list<const std::string>& filePaths) {
    return loadCubeMap(std::span{filePaths.begin(), filePaths.end()});
}

std::shared_ptr<Texture> TextureManager::get(const TextureHandle& handle) {
    return m_textures[handle];
}
