#pragma once
#include <vulkan/vulkan_core.h>

class Buffer {
   public:
    Buffer(const VkDevice& device, const VkPhysicalDevice& physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage,
           VkMemoryPropertyFlags properties);

   private:
    const VkDevice& m_device;
    const VkPhysicalDevice& m_physicalDevice;
    const VkDeviceSize m_size;

   public:
    void destroy() const;

    [[nodiscard]] VkDeviceSize getSize() const;
    [[nodiscard]] const VkBuffer& buffer() const;
    [[nodiscard]] const VkDeviceMemory& getMemory() const;

    void copyTo(const Buffer& dst, const VkCommandPool& commandPool, const VkQueue& queue) const;

   private:
    VkBuffer m_buffer = VK_NULL_HANDLE;
    VkDeviceMemory m_bufferMemory = VK_NULL_HANDLE;
};
