#pragma once

#include "../Model.h"

class Plane : public Model {
public:
    explicit Plane(TextureHandle textureHandle);

private:
    static std::unique_ptr<Mesh> _createPlaneMesh();
};

