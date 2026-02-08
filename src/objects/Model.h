#pragma once

#include "gfx/TextureManager.h"
#include "Node.h"

class Model {
public:
    explicit Model(std::unique_ptr<Node> rootNode);

    void destroy() const;

    // [[nodiscard]]
    // const Handle<Texture>& getTextureHandle() const;

    [[nodiscard]]
    Node* getRootNode() const;

    void translate(const glm::vec3& v) const;
    void rotate(float angle, const glm::vec3& axis) const;
    void scale(const glm::vec3& v) const;

private:
    std::unique_ptr<Node> m_rootNode;

    // Handle<Texture> m_textureHandle;
};
