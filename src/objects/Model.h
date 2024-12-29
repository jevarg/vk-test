#pragma once

#include "Mesh.h"
#include "common/Thing.h"
#include "gfx/vk/gpu_resources/Texture.h"

class Model : public Thing {
   public:
    explicit Model(const char* meshPath, Texture::ID textureID);
    Model(Mesh mesh, Texture::ID textureID);

    void destroy() const;

    [[nodiscard]]
    const Texture::ID& getTextureID() const;

    [[nodiscard]]
    const Mesh& getMesh() const;

   private:
    Texture::ID m_textureID;
    Mesh m_mesh;
};
