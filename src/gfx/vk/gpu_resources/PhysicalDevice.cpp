#include "PhysicalDevice.h"

#include <fmt/printf.h>

#include "gfx/vk/vkutil.h"

PhysicalDevice::PhysicalDevice(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR &surface)
    : m_underlying(physicalDevice) {
    uint32_t extensionCount = 0;
    VK_CHECK("Failed to count device extensions",
             vkEnumerateDeviceExtensionProperties(m_underlying, nullptr, &extensionCount, nullptr));

    m_extensions.resize(extensionCount);

    VK_CHECK("Failed to enumerate device extensions",
             vkEnumerateDeviceExtensionProperties(m_underlying, nullptr, &extensionCount, m_extensions.data()));

    vkGetPhysicalDeviceProperties(m_underlying, &m_properties);
    vkGetPhysicalDeviceFeatures(m_underlying, &m_features);

    m_findQueueFamilies(surface);
    m_querySwapChainSupport(surface);
}

void PhysicalDevice::m_findQueueFamilies(const VkSurfaceKHR &surface) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_underlying, &queueFamilyCount, nullptr);
    if (queueFamilyCount == 0) {
        throw std::runtime_error("Failed to get queue family count");
    }

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_underlying, &queueFamilyCount, queueFamilies.data());

    uint32_t i = 0;
    for (const VkQueueFamilyProperties &queueFamily : queueFamilies) {
        if (m_queueFamilies.isValid()) {
            break;  // We have found every required indices
        }

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            m_queueFamilies.graphicsFamily = i++;
            continue;
        }

        VkBool32 isSupported;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_underlying, i, surface, &isSupported);
        if (isSupported) {
            m_queueFamilies.presentFamily = i++;
            continue;
        }

        ++i;
    }

    if (!m_queueFamilies.isValid()) {
        throw std::runtime_error("Failed to get queue families");
    }
}

void PhysicalDevice::m_querySwapChainSupport(const VkSurfaceKHR &surface) {
    // Capabilities
    VK_CHECK("Failed to get surface capabilities",
             vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_underlying, surface, &m_swapChainSupport.capabilities));

    // Pixel formats
    uint32_t formatCount = 0;
    VK_CHECK("Failed to count surface pixel formats",
             vkGetPhysicalDeviceSurfaceFormatsKHR(m_underlying, surface, &formatCount, nullptr));
    if (formatCount) {
        m_swapChainSupport.formats.resize(formatCount);
        VK_CHECK("Failed to get surface pixel formats",
                 vkGetPhysicalDeviceSurfaceFormatsKHR(m_underlying, surface, &formatCount,
                                                      m_swapChainSupport.formats.data()));
    }

    // Present modes
    uint32_t presentModeCount = 0;
    VK_CHECK("Failed to count surface present modes",
             vkGetPhysicalDeviceSurfacePresentModesKHR(m_underlying, surface, &presentModeCount, nullptr));
    if (presentModeCount) {
        m_swapChainSupport.presentModes.resize(presentModeCount);
        VK_CHECK("Failed to get surface present modes",
                 vkGetPhysicalDeviceSurfacePresentModesKHR(m_underlying, surface, &presentModeCount,
                                                           m_swapChainSupport.presentModes.data()));
    }
}

bool PhysicalDevice::isSuitable() const {
    // Keeping it simple for now.
    // Later on, we can implement a simple device score system
    // e.g.:
    // https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/03_Physical_devices_and_queue_families.html#_base_device_suitability_checks
    bool isOk = m_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && m_features.geometryShader &&
                m_features.samplerAnisotropy && supportsExtensions({ VK_KHR_SWAPCHAIN_EXTENSION_NAME });

    if (isOk) {
        const bool swapChainAdequate = !m_swapChainSupport.formats.empty() && !m_swapChainSupport.presentModes.empty();
        isOk = m_queueFamilies.isValid() && swapChainAdequate;
    }

    fmt::println("{}: {}", m_properties.deviceName, isOk ? "OK" : "KO");
    return isOk;
}

bool PhysicalDevice::supportsExtensions(const std::vector<const char *> &requiredVKExtensions) const {
    for (const char *requiredExtension : requiredVKExtensions) {
        if (std::ranges::find_if(m_extensions.begin(), m_extensions.end(),
                                 [&requiredExtension](const VkExtensionProperties &props) {
                                     return strcmp(requiredExtension, props.extensionName) == 0;
                                 }) == m_extensions.end()) {
            return false;
        }
    }

    return true;
}

uint32_t PhysicalDevice::findMemoryType(const uint32_t type, const VkMemoryPropertyFlags properties) const {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_underlying, &memProperties);

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

const VkPhysicalDevice &PhysicalDevice::getUnderlying() const {
    return m_underlying;
}

const QueueFamilyIndices &PhysicalDevice::getQueueFamilyIndices() const {
    return m_queueFamilies;
}

const SwapChainSupportDetails &PhysicalDevice::getSwapChainSupportDetails() const {
    return m_swapChainSupport;
}
