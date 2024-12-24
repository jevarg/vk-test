#include "Model.h"

#include <fmt/base.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

Model::Model(const VulkanContext &vkContext, const char *meshPath, const char *texturePath)
    : m_vkContext(vkContext), m_texture(vkContext, texturePath), m_mesh(vkContext, meshPath) {
    fmt::println("Loaded model: {} ({} vertices)", meshPath, m_mesh.getIndices().size());
}

Model::Model(const VulkanContext &vkContext, Mesh mesh, Texture texture): m_vkContext(vkContext),
                                                                          m_texture(std::move(texture)),
                                                                          m_mesh(std::move(mesh)) {
}

void Model::destroy() const {
    m_mesh.destroy();
    m_texture.destroy();
}

const Texture &Model::getTexture() const {
    return m_texture;
}

const Mesh &Model::getMesh() const {
    return m_mesh;
}
