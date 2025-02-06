#pragma once

#include "Mesh.h"
#include "common/Thing.h"
#include "gfx/vk/gpu_resources/Texture.h"
#include "loaders/GLTFLoader.h"

class Model : public Thing {
   public:
    explicit Model(const GLTFLoader& loader);
    // Model(const char* meshPath, Texture::ID textureID);
    // Model(Mesh mesh, Texture::ID textureID);

    void destroy() const;

    [[nodiscard]]
    const Texture::ID& getTextureID() const;

    [[nodiscard]]
    const std::vector<std::shared_ptr<Mesh>>& getMeshes() const;

    void draw(const VkCommandBuffer& commandBuffer, const VkPipelineLayout& pipelineLayout) const;

   private:
    Texture::ID m_textureID;

    std::vector<std::shared_ptr<Mesh>> m_meshes;
    // std::vector<std::shared_ptr<Materials>> m_materials;
};
