#pragma once

#include <vulkan/vulkan_core.h>

class OneTimeCommand {
   public:
    explicit OneTimeCommand(const VkDevice& device, const VkCommandPool& commandPool, const VkQueue& queue);
    ~OneTimeCommand();

    VkCommandBuffer buffer = VK_NULL_HANDLE;

   private:
    const VkCommandPool& m_commandPool;
    const VkQueue& m_queue;
    const VkDevice& m_device;
};
