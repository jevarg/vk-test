#include "Model.h"

#include <fstream>
#include <json.hpp>

using json = nlohmann::json;

Model::Model(std::unique_ptr<Node> rootNode, const TextureHandle textureHandle)
    : m_textureHandle(textureHandle), m_rootNode(std::move(rootNode)) {}

void Model::destroy() const {
}

const TextureHandle& Model::getTextureHandle() const {
    return m_textureHandle;
}

Node* Model::getRootNode() const {
    return m_rootNode.get();
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
