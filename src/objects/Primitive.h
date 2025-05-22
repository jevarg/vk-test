#pragma once

#include <memory>
#include <vector>

#include "gfx/vk/types/Vertex.h"
#include "Material.h"

class Buffer;

class Primitive {
public:
    // explicit Mesh(const char* modelPath);

    Primitive(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    Primitive(Primitive&& other) noexcept = default;

    void destroy() const;

    [[nodiscard]]
    const Buffer& getVertexBuffer() const;

    [[nodiscard]]
    const Buffer& getIndexBuffer() const;

    [[nodiscard]]
    const std::vector<Vertex>& getVertices() const;

    [[nodiscard]]
    const std::vector<uint32_t>& getIndices() const;

    void setMaterial(std::unique_ptr<Material> material);

private:
    std::unique_ptr<Buffer> m_vertexBuffer;
    std::unique_ptr<Buffer> m_indexBuffer;

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;

    std::unique_ptr<Material> m_material;

    // uint32_t materialId;

    void m_createVertexBuffer();
    void m_createIndexBuffer();
};
