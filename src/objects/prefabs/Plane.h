#pragma once

#include "../Model.h"

class Plane : public Model {
public:
    explicit Plane(Handle<BasicMaterial> materialHandle);

private:
    static std::unique_ptr<Mesh> _createPlaneMesh(Handle<BasicMaterial> materialHandle);
};

