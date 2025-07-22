#pragma once

#include <list>
#include <memory>

#include "Mesh.h"
#include "common/Transform.h"

class Node : public std::enable_shared_from_this<Node> {
public:
    Node() = default;

    Node(const std::weak_ptr<Node>& parent, std::unique_ptr<Mesh> mesh);
    explicit Node(std::unique_ptr<Mesh> mesh);

    void setParent(const std::weak_ptr<Node>& parent);
    void addChild(const std::shared_ptr<Node>& other);

    void setMesh(std::unique_ptr<Mesh> mesh);

    const Transform& getTransform() const;
    void setPosition(const glm::vec3& v);

    void translate(const glm::vec3& v);
    void rotate(float angle, const glm::vec3& axis);
    void scale(const glm::vec3& v);

private:
    std::list<std::shared_ptr<Node>> m_children;
    std::weak_ptr<Node> m_parent;
    std::unique_ptr<Mesh> m_mesh;

    Transform m_transform;
};
