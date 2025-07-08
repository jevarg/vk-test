#include "Node.h"

// chatgpt node rootnode meshes and localtransform TRS

Node::Node(const std::shared_ptr<Node>& parent, std::unique_ptr<Mesh> mesh) : m_parent(parent), m_mesh(std::move(mesh)) {}

Node::Node(std::unique_ptr<Mesh> mesh) : m_mesh(std::move(mesh)) {}

void Node::setParent(const std::shared_ptr<Node>& parent) {
    m_parent = parent;
}

void Node::addChild(const std::shared_ptr<Node>& other) {
    other->setParent(std::shared_ptr<Node>(this));
    m_children.push_back(other);
}

void Node::setMesh(std::unique_ptr<Mesh> mesh) {
    m_mesh = std::move(mesh);
}
