#include "Model.h"

#include <fmt/base.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

Model::Model(const VulkanContext& vkContext, const char* modelPath, const char* texturePath)
    : m_vkContext(vkContext), m_texture(vkContext, texturePath), m_mesh(vkContext, modelPath) {
    fmt::println("Loaded model: {} ({} vertices)", modelPath, m_mesh.getIndices().size());
}

Model::Model(const VulkanContext& vkContext, const Mesh& mesh, const Texture& texture)
    : m_vkContext(vkContext), m_texture(texture), m_mesh() {}

void Model::destroy() const {
    m_mesh.destroy();
    m_texture.destroy();
}

const Texture& Model::getTexture() const {
    return m_texture;
}

const Mesh& Model::getMesh() const {
    return m_mesh;
}
