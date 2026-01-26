#include "Plane.h"

#include "objects/Primitive.h"
#include "objects/Mesh.h"

std::unique_ptr<Mesh> Plane::_createPlaneMesh() {
    static std::vector<Vertex> vertices = {
        {  { -0.5f, 0.0f, 0.5f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
        {   { 0.5f, 0.0f, 0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
        {  { 0.5f, 0.0f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },
        { { -0.5f, 0.0f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
    };

    static std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

    std::vector<Primitive> primitives;
    primitives.emplace_back(vertices, indices);

    return std::make_unique<Mesh>("Plane", std::move(primitives));
}

Plane::Plane(const TextureHandle textureHandle) : Model(std::make_unique<Node>(_createPlaneMesh()), textureHandle) {}
