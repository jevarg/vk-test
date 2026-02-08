#pragma once

#include "../Model.h"

class Cube : public Model {
    struct Materials {
        Handle<BasicMaterial> left;
        Handle<BasicMaterial> right;
        Handle<BasicMaterial> bottom;
        Handle<BasicMaterial> top;
        Handle<BasicMaterial> back;
        Handle<BasicMaterial> front;
    };

    static std::unique_ptr<Mesh> _createCubeMesh(const Materials& materials);

public:
    explicit Cube(const Materials& materials);
};
