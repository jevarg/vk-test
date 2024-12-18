#pragma once

#include <vulkan/vulkan_core.h>

#include "../types/VulkanContext.h"
#include "Texture.h"

class Buffer {
   public:
    Buffer(const VulkanContext& vkContext, VkDeviceSize size, VkBufferUsageFlags usage,
           VkMemoryPropertyFlags properties);

   private:
    const VulkanContext& m_vkContext;
    const VkDeviceSize m_size;

   public:
    void destroy() const;

    [[nodiscard]] VkDeviceSize getSize() const;
    [[nodiscard]] const VkBuffer& buffer() const;
    [[nodiscard]] const VkDeviceMemory& getMemory() const;

    void setMemory(const void* src, VkDeviceSize offset = 0, VkMemoryMapFlags flags = 0) const;
    void copyTo(const Buffer& dst, const VkCommandPool& commandPool, const VkQueue& queue) const;
    void copyTo(const Texture& texture, const VkCommandPool& commandPool, const VkQueue& queue) const;

   private:
    VkBuffer m_buffer = VK_NULL_HANDLE;
    VkDeviceMemory m_bufferMemory = VK_NULL_HANDLE;
};
