#pragma once

#include <string>
#include <vector>

#include "gfx/vk/gpu_resources/Buffer.h"
#include "gfx/vk/types/Vertex.h"

struct Submesh {
    uint32_t indexOffset;
    uint32_t indexCount;
};

class Mesh {
public:
    Mesh(std::string name, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    Mesh(std::string name, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, std::vector<Submesh> submeshes);

    // void draw(const VkCommandBuffer& commandBuffer, const VkPipelineLayout& pipelineLayout,
    //           const glm::mat4& worldTransform) const;

    [[nodiscard]]
    const Buffer& getVertexBuffer() const;

    [[nodiscard]]
    const Buffer& getIndexBuffer() const;

    [[nodiscard]]
    const std::vector<Submesh>& getSubmeshes() const;

private:
    Buffer m_createVertexBuffer(const std::vector<Vertex>& vertices) const;
 Buffer m_createIndexBuffer(const std::vector<uint32_t>& indices) const;

    std::string m_name;
    Buffer m_vertexBuffer;
    Buffer m_indexBuffer;
    std::vector<Submesh> m_submeshes;
};
