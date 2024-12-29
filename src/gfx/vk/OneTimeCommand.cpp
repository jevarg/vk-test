#include "OneTimeCommand.h"

#include "types/VulkanContext.h"
#include "vkutil.h"

OneTimeCommand::OneTimeCommand(const VkQueue& queue) : m_queue(queue) {
    const VulkanContext& vkContext = VulkanContext::get();

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vkContext.getCommandPool();
    allocInfo.commandBufferCount = 1;

    VK_CHECK("failed to allocate command buffer", vkAllocateCommandBuffers(vkContext.getDevice(), &allocInfo, &buffer));

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK("begin command buffer error", vkBeginCommandBuffer(buffer, &beginInfo));
}

OneTimeCommand::~OneTimeCommand() {
    VK_CHECK("end command buffer error", vkEndCommandBuffer(buffer));

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &buffer;

    VK_CHECK("failed to submit queue", vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE));
    VK_CHECK("failed to wait on graphics queue", vkQueueWaitIdle(m_queue));

    const VulkanContext& vkContext = VulkanContext::get();
    vkFreeCommandBuffers(vkContext.getDevice(), vkContext.getCommandPool(), 1, &buffer);
}
