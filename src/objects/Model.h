#pragma once

#include "Mesh.h"
#include "common/Thing.h"
#include "gfx/vk/gpu_resources/Texture.h"

class Model : public Thing {
   public:
    explicit Model(const VulkanContext& vkContext, const char* meshPath, const char* texturePath);
    Model(const VulkanContext& vkContext, Mesh mesh, Texture texture);

    void destroy() const;

    [[nodiscard]]
    const Texture& getTexture() const;

    [[nodiscard]]
    const Mesh& getMesh() const;

   private:
    const VulkanContext& m_vkContext;

    Texture m_texture;
    Mesh m_mesh;
};
