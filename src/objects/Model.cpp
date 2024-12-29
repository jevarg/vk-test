#include "Model.h"

#include <fmt/base.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

Model::Model(const char *meshPath, const Texture::ID textureID) : m_textureID(textureID), m_mesh(meshPath) {
    fmt::println("Loaded model: {} ({} vertices)", meshPath, m_mesh.getIndices().size());
}

Model::Model(Mesh mesh, const Texture::ID textureID) : m_textureID(textureID), m_mesh(std::move(mesh)) {}

void Model::destroy() const {
    m_mesh.destroy();
}

const Texture::ID &Model::getTextureID() const {
    return m_textureID;
}

const Mesh &Model::getMesh() const {
    return m_mesh;
}
