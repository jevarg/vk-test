#pragma once

#include <vulkan/vulkan_core.h>
#include <unordered_map>
#include <string>
#include <memory>
#include <span>

#include "Handle.h"

class Texture;
class TextureManager {
public:
    Handle<Texture> load(const std::string& filePath, VkDescriptorPool descriptorPool,
                         VkDescriptorSetLayout descriptorSetLayout);
    Handle<Texture> loadCubeMap(const std::span<const std::string>& filePaths, VkDescriptorPool descriptorPool,
                                VkDescriptorSetLayout descriptorSetLayout);
    Handle<Texture> loadCubeMap(const std::initializer_list<const std::string>& filePaths,
                                VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout);

    std::shared_ptr<Texture> get(const Handle<Texture>& handle);

private:
    std::unordered_map<Handle<Texture>, std::shared_ptr<Texture>, HandleHasher> m_textures;
    std::unordered_map<std::string, Handle<Texture>> m_textureCache;
};
