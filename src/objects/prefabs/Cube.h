#pragma once

#include "../Model.h"

class Cube : public Model {
    static std::unique_ptr<Mesh> _createCubeMesh();

public:
    explicit Cube(Texture::ID textureID);
    explicit Cube(TextureHandle textureHandle);
};
