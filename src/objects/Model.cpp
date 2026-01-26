#include "Model.h"

#include <fstream>
#include <json.hpp>

using json = nlohmann::json;

Model::Model(std::unique_ptr<Node> rootNode, const TextureHandle textureHandle)
    : m_textureHandle(textureHandle), m_rootNode(std::move(rootNode)) {}

void Model::destroy() const {
    // for (const auto& mesh : m_meshes) {
    //     mesh->destroy();
    // }
}

const TextureHandle& Model::getTextureHandle() const {
    return m_textureHandle;
}

void Model::draw(const VkCommandBuffer& commandBuffer, const VkPipelineLayout& pipelineLayout) const {
    m_rootNode->draw(commandBuffer, pipelineLayout, glm::mat4(1.0f));
}

void Model::translate(const glm::vec3& v) const {
    m_rootNode->translate(v);
}

void Model::rotate(const float angle, const glm::vec3& axis) const {
    m_rootNode->rotate(angle, axis);
}

void Model::scale(const glm::vec3& v) const {
    m_rootNode->scale(v);
}
