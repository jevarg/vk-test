#pragma once

#include "gfx/TextureManager.h"
#include "Node.h"

class Model {
public:
    Model(std::unique_ptr<Node> rootNode, TextureHandle textureHandle);

    void destroy() const;

    const TextureHandle& getTextureHandle() const;
    void draw(const VkCommandBuffer& commandBuffer, const VkPipelineLayout& pipelineLayout) const;

    void translate(const glm::vec3& v) const;
    void rotate(float angle, const glm::vec3& axis) const;
    void scale(const glm::vec3& v) const;

private:
    std::unique_ptr<Node> m_rootNode;

    TextureHandle m_textureHandle;
};
