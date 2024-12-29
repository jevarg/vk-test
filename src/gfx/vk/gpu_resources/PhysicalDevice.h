#pragma once

#include <optional>
#include <set>
#include <vector>
#include <vulkan/vulkan_core.h>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    [[nodiscard]]
    bool isValid() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }

    [[nodiscard]]
    std::set<uint32_t> getUnique() const {
        return { graphicsFamily.value(), presentFamily.value() };
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class PhysicalDevice {
public:
    explicit PhysicalDevice(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface);

    bool isSuitable() const;
    bool supportsExtensions(const std::vector<const char*>& requiredVKExtensions) const;
    uint32_t findMemoryType(uint32_t type, VkMemoryPropertyFlags properties) const;

    const VkPhysicalDevice& getUnderlying() const;
    const QueueFamilyIndices& getQueueFamilyIndices() const;
    const SwapChainSupportDetails& getSwapChainSupportDetails() const;

private:
    void m_findQueueFamilies(const VkSurfaceKHR& surface);
    void m_querySwapChainSupport(const VkSurfaceKHR& surface);

    VkPhysicalDevice m_underlying;

    std::vector<VkExtensionProperties> m_extensions;
    VkPhysicalDeviceProperties m_properties;
    VkPhysicalDeviceFeatures m_features;

    QueueFamilyIndices m_queueFamilies;
    SwapChainSupportDetails m_swapChainSupport;
};
