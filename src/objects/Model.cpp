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

void Model::draw(const VkCommandBuffer& commandBuffer, const VkPipelineLayout& pipelineLayout) const {
    const std::array buffers = { m_mesh.getVertexBuffer().buffer() };
    constexpr std::array<VkDeviceSize, buffers.size()> offsets = { 0 };

    vkCmdBindVertexBuffers(commandBuffer, 0, buffers.size(), buffers.data(), offsets.data());
    vkCmdBindIndexBuffer(commandBuffer, m_mesh.getIndexBuffer().buffer(), 0, VK_INDEX_TYPE_UINT32);

    const glm::mat4 constants = m_transform.getMatrix();
    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4),
                       &constants);
    vkCmdDrawIndexed(commandBuffer, m_mesh.getIndices().size(), 1, 0, 0, 0);
}