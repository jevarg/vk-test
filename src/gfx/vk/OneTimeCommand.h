#pragma once

#include <vulkan/vulkan_core.h>

class OneTimeCommand {
   public:
    explicit OneTimeCommand(const VkQueue& queue);
    ~OneTimeCommand();

    VkCommandBuffer buffer = VK_NULL_HANDLE;

   private:
    const VkQueue& m_queue;
};
