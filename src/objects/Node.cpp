#include "Node.h"
#include "Mesh.h"

// chatgpt node rootnode meshes and localtransform TRS

Node::Node(const std::weak_ptr<Node>& parent, const std::shared_ptr<Mesh>& mesh) : m_parent(parent), m_mesh(mesh) {}

Node::Node(const std::shared_ptr<Mesh>& mesh) : m_mesh(mesh) {}

void Node::setParent(const std::weak_ptr<Node>& parent) {
    m_parent = parent;
}

void Node::addChild(std::unique_ptr<Node> other) {
    other->setParent(weak_from_this());
    m_children.push_back(std::move(other));
}

void Node::setMesh(const std::shared_ptr<Mesh>& mesh) {
    m_mesh = mesh;
}

const Transform& Node::getTransform() const {
    return m_transform;
}

void Node::setPosition(const glm::vec3& v) {
    m_transform.setPosition(v);
}

void Node::translate(const glm::vec3& v) {
    m_transform.translate(v);
}

void Node::rotate(const float angle, const glm::vec3& axis) {
    m_transform.rotate(angle, axis);
}

void Node::scale(const glm::vec3& v) {
    m_transform.scale(v);
}

void Node::draw(const VkCommandBuffer& commandBuffer, const VkPipelineLayout& pipelineLayout, const glm::mat4& parentTransform) const {
    const glm::mat4 localMatrix = parentTransform * m_transform.getMatrix();
    m_mesh->draw(commandBuffer, pipelineLayout, localMatrix);
}
