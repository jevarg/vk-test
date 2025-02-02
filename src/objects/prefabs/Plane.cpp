#include "Plane.h"

const std::vector<Vertex> vertices = {
    {  { -0.5f, 0.0f, 0.5f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
    {   { 0.5f, 0.0f, 0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
    {  { 0.5f, 0.0f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },
    { { -0.5f, 0.0f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
};

const std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

// Plane::Plane(const Texture::ID textureID) : Model(Mesh(vertices, indices), textureID) {}
