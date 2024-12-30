#include "Buffer.h"

#include "gfx/vk/OneTimeCommand.h"
#include "gfx/vk/vkutil.h"

Buffer::Buffer(const VkDeviceSize size, const VkBufferUsageFlags usage, const VkMemoryPropertyFlags properties)
    : m_size(size) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = m_size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    const VulkanContext& vkContext = VulkanContext::get();

    VK_CHECK("failed to create vertex buffer", vkCreateBuffer(vkContext.getDevice(), &bufferInfo, nullptr, &m_buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vkContext.getDevice(), m_buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = vkContext.getPhysicalDevice().findMemoryType(memRequirements.memoryTypeBits, properties);

    VK_CHECK("failed to allocate vertex buffer memory",
             vkAllocateMemory(vkContext.getDevice(), &allocInfo, nullptr, &m_bufferMemory));

    vkBindBufferMemory(vkContext.getDevice(), m_buffer, m_bufferMemory, 0);
}

void Buffer::destroy() const {
    const VkDevice& device = VulkanContext::get().getDevice();

    vkFreeMemory(device, m_bufferMemory, nullptr);
    vkDestroyBuffer(device, m_buffer, nullptr);
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
    const VkDevice& device = VulkanContext::get().getDevice();
    void* data;

    vkMapMemory(device, m_bufferMemory, offset, m_size, flags, &data);
    memcpy(data, src, m_size);
    vkUnmapMemory(device, m_bufferMemory);
}

void Buffer::copyTo(const Buffer& dst) const {
    const OneTimeCommand cmd(VulkanContext::get().getGraphicsQueue());

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = m_size;

    vkCmdCopyBuffer(cmd.buffer, m_buffer, dst.buffer(), 1, &copyRegion);
}

void Buffer::copyTo(const Texture& texture) const {
    const OneTimeCommand cmd(VulkanContext::get().getGraphicsQueue());

    VkBufferImageCopy region{};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = texture.getImage().getExtent();

    vkCmdCopyBufferToImage(cmd.buffer, m_buffer, texture.getImage().getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                           &region);
}

void Buffer::update(const VkCommandBuffer& cmdBuffer, const void* data) const {
    vkCmdUpdateBuffer(cmdBuffer, m_buffer, 0, m_size, data);
}