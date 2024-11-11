#include "Buffer.h"

#include "vkutil.h"

Buffer::Buffer(const VkDevice& device, const VkPhysicalDevice& physicalDevice, const VkDeviceSize size,
               const VkBufferUsageFlags usage, const VkMemoryPropertyFlags properties)
    : m_device(device), m_physicalDevice(physicalDevice), m_size(size) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = m_size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK("failed to create vertex buffer", vkCreateBuffer(device, &bufferInfo, nullptr, &m_buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, m_buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

    VK_CHECK("failed to allocate vertex buffer memory", vkAllocateMemory(device, &allocInfo, nullptr, &m_bufferMemory));

    vkBindBufferMemory(device, m_buffer, m_bufferMemory, 0);
}

void Buffer::destroy() const {
    vkFreeMemory(m_device, m_bufferMemory, nullptr);
    vkDestroyBuffer(m_device, m_buffer, nullptr);
}

VkDeviceSize Buffer::getSize() const {
    return m_size;
}

const VkBuffer& Buffer::buffer() const {
    return m_buffer;
}

const VkDeviceMemory& Buffer::getMemory() const {
    return m_bufferMemory;
}

void Buffer::copyTo(const Buffer& dst, const VkCommandPool& commandPool, const VkQueue& queue) const {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    VK_CHECK("failed to allocate command buffer", vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer));

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK("begin command buffer error", vkBeginCommandBuffer(commandBuffer, &beginInfo));

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = m_size;
    vkCmdCopyBuffer(commandBuffer, m_buffer, dst.buffer(), 1, &copyRegion);

    VK_CHECK("end command buffer error", vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VK_CHECK("failed to submit queue", vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
    VK_CHECK("failed to wait on graphics queue", vkQueueWaitIdle(queue));

    vkFreeCommandBuffers(m_device, commandPool, 1, &commandBuffer);
}
