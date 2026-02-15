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

std::shared_ptr<Mesh> Node::getMesh() const {
    return m_mesh;
}

void Node::setMesh(const std::shared_ptr<Mesh>& mesh) {
    m_mesh = mesh;
}

const Transform& Node::getTransform() const {
    return m_transform;
}

glm::mat4 Node::getLocalMatrix() const {
    return m_transform.getMatrix();
}

glm::mat4 Node::getWorldMatrix() const {
    const auto parent = m_parent.lock();
    if (!parent) {
        return m_transform.getMatrix();
    }

    return parent->getWorldMatrix() * m_transform.getMatrix();
}

void Node::setPosition(const glm::vec3& v) {
    m_transform.setPosition(v);
}

void Node::translate(const glm::vec3& v) {
    m_transform.translate(v);
}

void Node::translateLocal(const glm::vec3& v) {
    m_transform.translateLocal(v);
}

void Node::rotate(const float angle, const glm::vec3& axis) {
    m_transform.rotate(angle, axis);
}

void Node::rotateLocal(const float angle, const glm::vec3& axis) {
    m_transform.rotateLocal(angle, axis);
}

void Node::scale(const glm::vec3& v) {
    m_transform.scale(v);
}

// void Node::draw(const VkCommandBuffer& commandBuffer, const VkPipelineLayout& pipelineLayout, const glm::mat4& parentTransform) const {
//     const glm::mat4 localMatrix = parentTransform * m_transform.getMatrix();
    // m_mesh->draw(commandBuffer, pipelineLayout, localMatrix);
// }
