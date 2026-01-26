#include "Cube.h"
#include "objects/Mesh.h"

constexpr float uvX = 0.25;
constexpr float uvY = 1.0f / 3;

std::unique_ptr<Mesh> Cube::_createCubeMesh() {
    static const std::vector<Vertex> vertices = {
        { { -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } },
        { { -1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } },
        { { -1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } },
        { { -1.0f, 1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } },
        { { 1.0f, 1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } },
        { { 1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } },
        { { 1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } },
        { { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } },
    };

    //    .3------4
    //  .' |    .'|
    // 2---+--7'  |
    // |   |  |   |
    // |  ,0--+---5
    // |.'    | .'
    // 1------6'


    static const std::vector<uint32_t> indices = {
        // -x
        0, 1, 2, 2, 3, 0,
        // +x
        6, 5, 4, 4, 7, 6,
        // -y
        6, 1, 0, 0, 5, 6,
        // +y
        2, 7, 4, 4, 3, 2,
        // -z
        0, 3, 4, 4, 5, 0,
        // +z
        6, 7, 2, 2, 1, 6,
    };

    std::vector<Primitive> primitives;
    primitives.emplace_back(vertices, indices);

    return std::make_unique<Mesh>("Cube", std::move(primitives));
}

Cube::Cube(const TextureHandle textureHandle) : Model(std::make_unique<Node>(_createCubeMesh()), textureHandle) {}
