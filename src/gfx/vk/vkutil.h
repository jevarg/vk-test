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

// inline uint32_t findMemoryType(const VkPhysicalDevice& physicalDevice, const uint32_t type,
//                                const VkMemoryPropertyFlags properties) {
//     VkPhysicalDeviceMemoryProperties memProperties;
//     vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
//
//     for (int i = 0; i < memProperties.memoryTypeCount; ++i) {
//         if (!(type & (1 << i))) {
//             continue;
//         }
//
//         if ((memProperties.memoryTypes[i].propertyFlags & properties) != properties) {
//             continue;
//         }
//
//         return i;
//     }
//
//     throw std::runtime_error("unable to find memory type!");
// }

// bool isDeviceSuitable(const VkPhysicalDevice& physicalDevice) {
//     VkPhysicalDeviceProperties properties;
//     vkGetPhysicalDeviceProperties(physicalDevice, &properties);
//
//     VkPhysicalDeviceFeatures features;
//     vkGetPhysicalDeviceFeatures(physicalDevice, &features);
//
//     // Keeping it simple for now.
//     // Later on, we can implement a simple device score system
//     // e.g.:
//     // https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/03_Physical_devices_and_queue_families.html#_base_device_suitability_checks
//     bool isSuitable = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && features.geometryShader &&
//                       features.samplerAnisotropy && m_checkDeviceExtensionSupport(physicalDevice);
//
//     if (isSuitable) {
//         const QueueFamilyIndices queueFamilyIndices = m_findQueueFamilies(physicalDevice);
//         const SwapChainSupportDetails swapChainSupport = m_querySwapChainSupport(physicalDevice);
//
//         bool swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
//
//         isSuitable = queueFamilyIndices.isValid() && swapChainAdequate;
//     }
//
//     fmt::println("{}: {}", properties.deviceName, isSuitable ? "OK" : "KO");
//     return isSuitable;
// }