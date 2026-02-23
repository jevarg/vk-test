#include "SkyboxMesh.h"

#include "objects/Mesh.h"

std::unique_ptr<Mesh> SkyboxMesh::_createMesh(const Handle<BasicMaterial>& material) {
    static const std::vector<Vertex> vertices = {
        // -x face (left)
        { { -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } },
        { { -1.0f, -1.0f,  1.0f }, { 1.0f, 1.0f, 1.0f } },
        { { -1.0f,  1.0f,  1.0f }, { 1.0f, 1.0f, 1.0f } },
        { { -1.0f,  1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } },

        // +x face (right)
        { {  1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } },
        { {  1.0f,  1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } },
        { {  1.0f,  1.0f,  1.0f }, { 1.0f, 1.0f, 1.0f } },
        { {  1.0f, -1.0f,  1.0f }, { 1.0f, 1.0f, 1.0f } },

        // -y face (bottom)
        { { -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } },
        { {  1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } },
        { {  1.0f, -1.0f,  1.0f }, { 1.0f, 1.0f, 1.0f } },
        { { -1.0f, -1.0f,  1.0f }, { 1.0f, 1.0f, 1.0f } },

        // +y face (top)
        { { -1.0f,  1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } },
        { { -1.0f,  1.0f,  1.0f }, { 1.0f, 1.0f, 1.0f } },
        { {  1.0f,  1.0f,  1.0f }, { 1.0f, 1.0f, 1.0f } },
        { {  1.0f,  1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } },

        // -z face (back)
        { { -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } },
        { { -1.0f,  1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } },
        { {  1.0f,  1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } },
        { {  1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } },

        // +z face (front)
        { { -1.0f, -1.0f,  1.0f }, { 1.0f, 1.0f, 1.0f } },
        { {  1.0f, -1.0f,  1.0f }, { 1.0f, 1.0f, 1.0f } },
        { {  1.0f,  1.0f,  1.0f }, { 1.0f, 1.0f, 1.0f } },
        { { -1.0f,  1.0f,  1.0f }, { 1.0f, 1.0f, 1.0f } },
    };

    static const std::vector<uint32_t> indices = {
        // -x
        0, 1, 2, 2, 3, 0,
        // +x
        4, 5, 6, 6, 7, 4,
        // -y
        8, 9, 10, 10, 11, 8,
        // +y
        12, 13, 14, 14, 15, 12,
        // -z
        16, 17, 18, 18, 19, 16,
        // +z
        20, 21, 22, 22, 23, 20,
    };

    return std::make_unique<Mesh>("Skybox", vertices, indices, std::vector{ material });
}

SkyboxMesh::SkyboxMesh(const Handle<BasicMaterial>& material)
    : Model(std::make_unique<Node>(_createMesh(material))) {}
