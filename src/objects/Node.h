#pragma once
#include <list>
#include <memory>

class Mesh;

class Node {
public:
    Node() = default;
    Node(Node* parent, Mesh* mesh);

    void setParent(Node* parent);
    void setMesh(Mesh* mesh);
    void addChild(std::unique_ptr<Node> other);

private:
    std::list<std::unique_ptr<Node>> m_children;
    Node* m_parent = nullptr;
    Mesh* m_mesh = nullptr;
};
