#pragma once

#include <vulkan/vulkan_core.h>
#include <unordered_map>
#include <string>
#include <memory>
#include <span>

class TextureHandle {
    inline static uint32_t m_lastID = 0;

public:
    uint32_t handle = ++m_lastID;

    bool operator==(const TextureHandle& other) const {
        return handle == other.handle;
    }
};

struct TextureHandleHasher {
    std::size_t operator()(const TextureHandle& k) const {
        return std::hash<uint32_t>()(k.handle);
    }
};

class Texture;
class TextureManager {
public:
    explicit TextureManager(VkDescriptorPool descriptorPool);

    TextureHandle loadTexture(const std::string& filePath);
    TextureHandle loadCubeMap(const std::span<const std::string>& filePaths);
    TextureHandle loadCubeMap(const std::initializer_list<const std::string>& filePaths);

    std::shared_ptr<Texture> get(const TextureHandle& handle);

private:
    VkDescriptorPool m_descriptorPool = nullptr;
    VkDescriptorSetLayout m_descriptorSetLayout = nullptr;

    std::unordered_map<TextureHandle, std::shared_ptr<Texture>, TextureHandleHasher> m_textures;
    std::unordered_map<std::string, TextureHandle> m_textureCache;
};
