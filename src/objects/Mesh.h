#pragma once

#include <memory>
#include <vector>

#include "gfx/vk/gpu_resources/Buffer.h"
#include "gfx/vk/types/Vertex.h"

class Mesh {
   public:
    // explicit Mesh(const char* modelPath);

    Mesh(const char* name, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    Mesh(Mesh&& other) noexcept = default;

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
    std::string m_name;
    std::unique_ptr<Buffer> m_vertexBuffer;
    std::unique_ptr<Buffer> m_indexBuffer;

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;

    void m_createVertexBuffer();
    void m_createIndexBuffer();

};
