#include "Cube.h"

constexpr float uvX = 0.25;
constexpr float uvY = 1.0f / 3;

const std::vector<Vertex> vertices = {
    { { -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } },
    {  { -1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } },
    {   { -1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } },
    {  { -1.0f, 1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } },
    {   { 1.0f, 1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } },
    {  { 1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } },
    {   { 1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } },
    {    { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } },
};

//    .3------4
//  .' |    .'|
// 2---+--7'  |
// |   |  |   |
// |  ,0--+---5
// |.'    | .'
// 1------6'


const std::vector<uint32_t> indices = {
    // -x
    0, 1, 2, 2, 3, 0,
    // +x
    6, 5, 4, 4, 7, 6,
    // -y
    6, 1, 0, 0, 5, 6,
    // +y
    7, 2, 3, 3, 4, 7,
    // -z
    5, 0, 3, 3, 4, 5,
    // +z
    6, 1, 2, 2, 7, 6,
};

Cube::Cube(const Texture::ID textureID) : Model(Mesh(vertices, indices), textureID) {}
