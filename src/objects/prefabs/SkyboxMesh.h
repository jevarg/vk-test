#pragma once

#include "objects/Model.h"

class SkyboxMesh : public Model {
    static std::unique_ptr<Mesh> _createMesh(const Handle<BasicMaterial>& material);

public:
    explicit SkyboxMesh(const Handle<BasicMaterial>& material);
};