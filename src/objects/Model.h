#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>

#include "gfx/vk/gpu_resources/Buffer.h"
#include "gfx/vk/gpu_resources/Texture.h"
#include "gfx/vk/types/Vertex.h"

class Model {
   public:
    explicit Model(const VulkanContext& vkContext, const char* modelPath, const char* texturePath);

    void destroy() const;

    [[nodiscard]]
    const Texture& getTexture() const;

    [[nodiscard]]
    const Buffer& getVertexBuffer() const;

    [[nodiscard]]
    const Buffer& getIndexBuffer() const;

    [[nodiscard]]
    const std::vector<Vertex>& getVertices() const;

    [[nodiscard]]
    const std::vector<uint32_t>& getIndices() const;

   private:
    const VulkanContext& m_vkContext;

    Texture m_texture;
    std::unique_ptr<Buffer> m_vertexBuffer;
    std::unique_ptr<Buffer> m_indexBuffer;

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;

    void m_createVertexBuffer(VkCommandPool commandPool, VkQueue queue);
    void m_createIndexBuffer(VkCommandPool commandPool, VkQueue queue);
};
