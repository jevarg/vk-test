#pragma once

#include "../Model.h"

class Cube : public Model {
    static std::unique_ptr<Mesh> _createCubeMesh();

public:
    explicit Cube(TextureHandle textureHandle);
};
