#include "Mesh.h"

#include <tiny_obj_loader.h>

#include <stdexcept>

Mesh::Mesh(const VulkanContext& vkContext, const char* modelPath) {
    tinyobj::attrib_t attrib;

    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;

    if (!LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath)) {
        throw std::runtime_error(warn + err);
    }

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2],
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
            };

            vertex.color = { 1.0f, 1.0f, 1.0f };

            m_vertices.push_back(vertex);

            // TODO: indices are not used for now;
            m_indices.push_back(m_indices.size());
        }
    }

    m_createVertexBuffer(vkContext);
    m_createIndexBuffer(vkContext);
}

Mesh::Mesh(const VulkanContext &vkContext, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices) {
    m_vertices = vertices;
    m_indices = indices;

    m_createVertexBuffer(vkContext);
    m_createIndexBuffer(vkContext);
}

void Mesh::destroy() const {
    m_vertexBuffer->destroy();
    m_indexBuffer->destroy();
}

void Mesh::m_createVertexBuffer(const VulkanContext& vkContext) {
    const size_t bufferSize = sizeof(m_vertices[0]) * m_vertices.size();
    const Buffer stagingBuffer(vkContext, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    m_vertexBuffer = std::make_unique<Buffer>(vkContext, bufferSize,
                                              VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    stagingBuffer.setMemory(m_vertices.data());
    stagingBuffer.copyTo(*m_vertexBuffer, vkContext.commandPool, vkContext.graphicsQueue);
    stagingBuffer.destroy();
}

void Mesh::m_createIndexBuffer(const VulkanContext& vkContext) {
    const size_t bufferSize = sizeof(m_indices[0]) * m_indices.size();
    const Buffer stagingBuffer(vkContext, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    m_indexBuffer = std::make_unique<Buffer>(vkContext, bufferSize,
                                             VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    void* data;
    vkMapMemory(vkContext.device, stagingBuffer.getMemory(), 0, stagingBuffer.getSize(), 0, &data);
    memcpy(data, m_indices.data(), stagingBuffer.getSize());
    vkUnmapMemory(vkContext.device, stagingBuffer.getMemory());

    stagingBuffer.copyTo(*m_indexBuffer, vkContext.commandPool, vkContext.graphicsQueue);
    stagingBuffer.destroy();
}

const Buffer& Mesh::getVertexBuffer() const {
    return *m_vertexBuffer;
}

const Buffer& Mesh::getIndexBuffer() const {
    return *m_indexBuffer;
}

const std::vector<Vertex>& Mesh::getVertices() const {
    return m_vertices;
}

const std::vector<uint32_t>& Mesh::getIndices() const {
    return m_indices;
}
