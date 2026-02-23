#include "Mesh.h"

Mesh::Mesh(std::string name, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
           const std::vector<Handle<BasicMaterial>>& materials)
    : Mesh(std::move(name), vertices, indices, materials,
           {{ 0, static_cast<uint32_t>(indices.size()), 0 }}) {}

Mesh::Mesh(std::string name, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
           const std::vector<Handle<BasicMaterial>>& materials, const std::vector<Submesh>& submeshes)
    : m_name(std::move(name)),
      m_vertexBuffer(m_createVertexBuffer(vertices)),
      m_indexBuffer(m_createIndexBuffer(indices)),
      m_materials(materials),
      m_submeshes(submeshes) {}

const Buffer& Mesh::getVertexBuffer() const {
    return m_vertexBuffer;
}

const Buffer& Mesh::getIndexBuffer() const {
    return m_indexBuffer;
}

const std::vector<Submesh>& Mesh::getSubmeshes() const {
    return m_submeshes;
}

const std::vector<Handle<BasicMaterial>>& Mesh::getMaterials() const {
    return m_materials;
}

Buffer Mesh::m_createVertexBuffer(const std::vector<Vertex>& vertices) const {
    const size_t bufferSize = sizeof(vertices[0]) * vertices.size();
    const Buffer stagingBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    Buffer buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    stagingBuffer.setMemory(vertices.data());
    stagingBuffer.copyTo(m_vertexBuffer);
    stagingBuffer.destroy();

    return buffer;
}

Buffer Mesh::m_createIndexBuffer(const std::vector<uint32_t>& indices) const {
    const size_t bufferSize = sizeof(indices[0]) * indices.size();
    const Buffer stagingBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    Buffer buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    stagingBuffer.setMemory(indices.data());
    stagingBuffer.copyTo(m_indexBuffer);
    stagingBuffer.destroy();

    return buffer;
}