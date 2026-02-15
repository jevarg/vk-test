#pragma once

#include <list>
#include <memory>

#include "Mesh.h"
#include "common/Transform.h"

class Node : public std::enable_shared_from_this<Node> {
public:
    Node() = default;

    Node(const std::weak_ptr<Node>& parent, const std::shared_ptr<Mesh>& mesh);
    explicit Node(const std::shared_ptr<Mesh>& mesh);

    void setParent(const std::weak_ptr<Node>& parent);
    void addChild(std::unique_ptr<Node> other);

    [[nodiscard]]
    std::shared_ptr<Mesh> getMesh() const;
    void setMesh(const std::shared_ptr<Mesh>& mesh);

    [[nodiscard]]
    const Transform& getTransform() const;

    [[nodiscard]]
    glm::mat4 getLocalMatrix() const;

    [[nodiscard]]
    glm::mat4 getWorldMatrix() const;

    void setPosition(const glm::vec3& v);

    void translate(const glm::vec3& v);
    void translateLocal(const glm::vec3& v);

    void rotate(float angle, const glm::vec3& axis);
    void rotateLocal(float angle, const glm::vec3& axis);

    void scale(const glm::vec3& v);

private:
    std::list<std::unique_ptr<Node>> m_children;
    std::weak_ptr<Node> m_parent;
    std::shared_ptr<Mesh> m_mesh;

    Transform m_transform;
};
