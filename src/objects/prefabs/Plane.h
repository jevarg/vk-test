#pragma once
#include "../Model.h"

class Plane : public Model {
private:
    static std::unique_ptr<Mesh> _createPlaneMesh();
public:
    explicit Plane(Texture::ID textureID);
};

