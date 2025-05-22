#include "Node.h"

// chatgpt node rootnode meshes and localtransform TRS

Node::Node(Node* parent = nullptr, Mesh* mesh = nullptr): m_parent(parent), m_mesh(mesh) {}

void Node::setParent(Node* parent) {
    m_parent = parent;
}

void Node::setMesh(Mesh* mesh) {
    m_mesh = mesh;
}

void Node::addChild(std::unique_ptr<Node> other) {
    other->setParent(this);
    m_children.push_back(std::move(other));
}
