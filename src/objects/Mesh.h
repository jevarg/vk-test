#pragma once

#include <memory>
#include <vector>

#include "gfx/vk/gpu_resources/Buffer.h"
#include "gfx/vk/types/Vertex.h"

class Mesh {
   public:
    explicit Mesh(const VulkanContext& vkContext, const char* modelPath);
    void destroy() const;

    [[nodiscard]]
    const Buffer& getVertexBuffer() const;

    [[nodiscard]]
    const Buffer& getIndexBuffer() const;

    [[nodiscard]]
    const std::vector<Vertex>& getVertices() const;

    [[nodiscard]]
    const std::vector<uint32_t>& getIndices() const;

   private:
    std::unique_ptr<Buffer> m_vertexBuffer;
    std::unique_ptr<Buffer> m_indexBuffer;

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;

    void m_createVertexBuffer(const VulkanContext& vkContext);
    void m_createIndexBuffer(const VulkanContext& vkContext);
};
