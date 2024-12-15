#pragma once

#include <vulkan/vulkan_core.h>

struct VulkanContext {
    VkDevice device;
    VkPhysicalDevice physicalDevice;

    VkCommandPool commandPool;
    VkQueue graphicsQueue;
};
