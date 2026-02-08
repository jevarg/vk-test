#include "TextureManager.h"

#include "vk/gpu_resources/Texture.h"
#include "vk/vkutil.h"

// TextureManager::~TextureManager() {
//     for (const auto pair : m_textures) {
//         pair.second->destroy();
//     }
// }

Handle<Texture> TextureManager::load(const std::string& filePath, VkDescriptorPool descriptorPool,
                                     VkDescriptorSetLayout descriptorSetLayout) {
    const auto it = m_textureCache.find(filePath);
    if (it != m_textureCache.end()) {
        return it->second;
    }

    Handle<Texture> handle;
    m_textures.emplace(handle, std::make_shared<Texture>(filePath, descriptorPool, descriptorSetLayout));

    fmt::println("[{}] Loaded {}", handle.id, filePath);

    return handle;
}

Handle<Texture> TextureManager::loadCubeMap(const std::span<const std::string>& filePaths,
                                            VkDescriptorPool descriptorPool,
                                            VkDescriptorSetLayout descriptorSetLayout) {
    fmt::println("Loading Cube texture... (Warning: not cached)");

    Handle<Texture> handle;
    m_textures.emplace(handle, std::make_shared<Texture>(filePaths, descriptorPool, descriptorSetLayout));

    for (const auto& path : filePaths) {
        fmt::println("[{}] Loaded {}", handle.id, path);
    }

    return handle;
}

Handle<Texture> TextureManager::loadCubeMap(const std::initializer_list<const std::string>& filePaths,
                                            VkDescriptorPool descriptorPool,
                                            VkDescriptorSetLayout descriptorSetLayout) {
    return loadCubeMap(std::span{ filePaths.begin(), filePaths.end() }, descriptorPool, descriptorSetLayout);
}

std::shared_ptr<Texture> TextureManager::get(const Handle<Texture>& handle) {
    return m_textures[handle];
}
