#pragma once

#include <vulkan/vulkan_core.h>

#include <unordered_map>

#include "Handle.h"
#include "vk/pipeline/Pipeline.h"

class TextureManager;
class Pipeline;
class BasicMaterial;
class Texture;

struct MaterialData{
    std::string name{"Unnamed material"};
    std::string baseColorTexture;
};

class MaterialManager {
public:
    explicit MaterialManager(TextureManager& textureManager, uint32_t maxSize);

    Handle<BasicMaterial> load(const MaterialData& data);

    [[nodiscard]]
    std::shared_ptr<BasicMaterial> get(const Handle<BasicMaterial>& handle);

private:
    TextureManager& m_textureManager;

    VkDescriptorPool m_descriptorPool = nullptr;
    VkDescriptorSetLayout m_descriptorSetLayout = nullptr;

    std::unordered_map<Handle<BasicMaterial>, std::shared_ptr<BasicMaterial>, HandleHasher> m_materials;
    // std::unordered_map<std::string, Handle<BasicMaterial>> m_materialCache;

    Handle<BasicMaterial> m_defaultMaterial;
};
