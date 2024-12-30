#pragma once

#include <vulkan/vulkan_core.h>

#include "Texture.h"

class Buffer {
   public:
    Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

    void destroy() const;

    [[nodiscard]]
    VkDeviceSize getSize() const;

    [[nodiscard]]
    const VkBuffer& buffer() const;

    [[nodiscard]]
    const VkDeviceMemory& getMemory() const;

    void setMemory(const void* src, VkDeviceSize offset = 0, VkMemoryMapFlags flags = 0) const;
    void update(const VkCommandBuffer& cmdBuffer, const void* data) const;
    void copyTo(const Buffer& dst) const;
    void copyTo(const Texture& texture) const;

   private:
    const VkDeviceSize m_size;

    VkBuffer m_buffer = VK_NULL_HANDLE;
    VkDeviceMemory m_bufferMemory = VK_NULL_HANDLE;
};
