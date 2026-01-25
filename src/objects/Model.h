#pragma once

#include "Node.h"
#include "gfx/vk/gpu_resources/Texture.h"
#include "loaders/GLTFLoader.h"

class Model {
public:
    Model(std::unique_ptr<Node> rootNode, Texture::ID textureID);
    explicit Model(const GLTFLoader& loader);
    // Model(const char* meshPath, Texture::ID textureID);
    // Model(Mesh mesh, Texture::ID textureID);

    void destroy() const;

    [[nodiscard]]
    const Texture::ID& getTextureID() const;

    // [[nodiscard]]œ
    // const std::vector<std::shared_ptr<Mesh>>& getMeshes() const;

    void draw(const VkCommandBuffer& commandBuffer, const VkPipelineLayout& pipelineLayout) const;

    void translate(const glm::vec3& v) const;
    void rotate(float angle, const glm::vec3& axis) const;
    void scale(const glm::vec3& v) const;

private:
    Texture::ID m_textureID;

    std::unique_ptr<Node> m_rootNode;
    // std::vector<std::shared_ptr<Mesh>> m_meshes;
    // std::unordered_map<Material> m_materials;
    // std::vector<std::shared_ptr<Material>> m_materials;
};
