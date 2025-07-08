#pragma once
#include <list>
#include <memory>

class Mesh;

class Node {
public:
    Node() = default;

    Node(const std::shared_ptr<Node>& parent, std::unique_ptr<Mesh> mesh);
    explicit Node(std::unique_ptr<Mesh> mesh);

    void setParent(const std::shared_ptr<Node>& parent);
    void addChild(const std::shared_ptr<Node>& other);

    void setMesh(std::unique_ptr<Mesh> mesh);

private:
    std::list<std::shared_ptr<Node>> m_children;
    std::shared_ptr<Node> m_parent;
    std::unique_ptr<Mesh> m_mesh;
};
