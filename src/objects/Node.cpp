#include "Node.h"
#include "Mesh.h"

// chatgpt node rootnode meshes and localtransform TRS

Node::Node(const std::weak_ptr<Node>& parent, std::unique_ptr<Mesh> mesh) : m_parent(parent), m_mesh(std::move(mesh)) {}

Node::Node(std::unique_ptr<Mesh> mesh) : m_mesh(std::move(mesh)) {}

void Node::setParent(const std::weak_ptr<Node>& parent) {
    m_parent = parent;
}

void Node::addChild(const std::shared_ptr<Node>& other) {
    other->setParent(weak_from_this());
    m_children.push_back(other);
}

void Node::setMesh(std::unique_ptr<Mesh> mesh) {
    m_mesh = std::move(mesh);
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
