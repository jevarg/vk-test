#pragma once

#include <stdexcept>
#include <fmt/format.h>
#include <vulkan/vk_enum_string_helper.h>

#define VK_CHECK(msg, res) handleVKError(msg, res, __FILE__, __LINE__)

inline void handleVKError(const char* msg, const VkResult res, const char* file, const int line) {
    if (res == VK_SUCCESS) {
        return;
    }

    throw std::runtime_error(fmt::format("[{}:{}]: {} ({})", file, line, msg, string_VkResult(res)));
}

inline uint32_t findMemoryType(const VkPhysicalDevice& physicalDevice, const uint32_t type,
                               const VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (int i = 0; i < memProperties.memoryTypeCount; ++i) {
        if (!(type & (1 << i))) {
            continue;
        }

        if ((memProperties.memoryTypes[i].propertyFlags & properties) != properties) {
            continue;
        }

        return i;
    }

    throw std::runtime_error("unable to find memory type!");
}