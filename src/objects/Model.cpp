#include "Model.h"

#include <fmt/base.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

Model::Model(const VkDevice device, const VkPhysicalDevice physicalDevice, const VkCommandPool commandPool,
             const VkQueue queue, const char* modelPath, const char* texturePath)
    : m_device(device),
      m_physicalDevice(physicalDevice),
      m_texture(device, physicalDevice, commandPool, queue, texturePath) {
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
            m_indices.push_back(m_indices.size());
        }
    }

    m_createVertexBuffer(commandPool, queue);
    m_createIndexBuffer(commandPool, queue);

    fmt::println("Loaded model: {}", modelPath);
}

void Model::destroy() const {
    m_vertexBuffer->destroy();
    m_indexBuffer->destroy();

    m_texture.destroy();
}

const Texture& Model::getTexture() const {
    return m_texture;
}

const Buffer& Model::getVertexBuffer() const {
    return *m_vertexBuffer;
}

const Buffer& Model::getIndexBuffer() const {
    return *m_indexBuffer;
}

const std::vector<Vertex>& Model::getVertices() const {
    return m_vertices;
}

const std::vector<uint32_t>& Model::getIndices() const {
    return m_indices;
}

void Model::m_createVertexBuffer(const VkCommandPool commandPool, const VkQueue queue) {
    const size_t bufferSize = sizeof(m_vertices[0]) * m_vertices.size();
    const Buffer stagingBuffer(m_device, m_physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    m_vertexBuffer = std::make_unique<Buffer>(m_device, m_physicalDevice, bufferSize,
                                              VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    stagingBuffer.setMemory(m_vertices.data());
    stagingBuffer.copyTo(*m_vertexBuffer, commandPool, queue);
    stagingBuffer.destroy();
}

void Model::m_createIndexBuffer(const VkCommandPool commandPool, const VkQueue queue) {
    const size_t bufferSize = sizeof(m_indices[0]) * m_indices.size();
    const Buffer stagingBuffer(m_device, m_physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    m_indexBuffer = std::make_unique<Buffer>(m_device, m_physicalDevice, bufferSize,
                                             VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    void* data;
    vkMapMemory(m_device, stagingBuffer.getMemory(), 0, stagingBuffer.getSize(), 0, &data);
    memcpy(data, m_indices.data(), stagingBuffer.getSize());
    vkUnmapMemory(m_device, stagingBuffer.getMemory());

    stagingBuffer.copyTo(*m_indexBuffer, commandPool, queue);
    stagingBuffer.destroy();
}

