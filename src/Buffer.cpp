#include "Buffer.h"

#include "OneTimeCommand.h"
#include "vkutil.h"

Buffer::Buffer(const VkDevice& device, const VkPhysicalDevice& physicalDevice, const VkDeviceSize size,
               const VkBufferUsageFlags usage, const VkMemoryPropertyFlags properties)
    : m_device(device), m_size(size) {
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

void Buffer::setMemory(const void* src, const VkDeviceSize offset, const VkMemoryMapFlags flags) const {
    void* data;

    vkMapMemory(m_device, m_bufferMemory, offset, m_size, flags, &data);
    memcpy(data, src, m_size);
    vkUnmapMemory(m_device, m_bufferMemory);
}

void Buffer::copyTo(const Buffer& dst, const VkCommandPool& commandPool, const VkQueue& queue) const {
    OneTimeCommand cmd(m_device, commandPool, queue);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = m_size;

    vkCmdCopyBuffer(cmd.buffer, m_buffer, dst.buffer(), 1, &copyRegion);
}

void Buffer::copyTo(const Texture& texture, VkCommandPool const& commandPool, VkQueue const& queue) const {
    OneTimeCommand cmd(m_device, commandPool, queue);

    VkBufferImageCopy region{};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = texture.getExtent();

    vkCmdCopyBufferToImage(cmd.buffer, m_buffer, texture.getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}
