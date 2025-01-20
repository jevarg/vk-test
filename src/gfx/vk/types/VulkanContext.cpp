#include "VulkanContext.h"

#include <vector>
#include <fmt/base.h>

#include "../vkutil.h"

VulkanContext &VulkanContext::get() {
    static VulkanContext shared;
    return shared;
}

void VulkanContext::init(const VkInstance &vkInstance, const VkSurfaceKHR &vkSurface) {
    m_vkInstance = vkInstance;
    m_pickPhysicalDevice(vkSurface);
    m_createLogicalDevice();
    m_createCommandPool();

    m_initialized = true;
}

void VulkanContext::destroy() {
    fmt::println("Destroying vulkan context");
    if (!m_initialized) {
        fmt::println("Skipped (not initialized)");
        return;
    }

    vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    vkDestroyDevice(m_device, nullptr);

    m_initialized = false;
    fmt::println("Done");
}

bool VulkanContext::isInitialized() const {
    return m_initialized;
}

const PhysicalDevice& VulkanContext::getPhysicalDevice() const {
    return *m_physicalDevice;
}

const VkDevice& VulkanContext::getDevice() const {
    return m_device;
}

const VkCommandPool & VulkanContext::getCommandPool() const {
    return m_commandPool;
}

const VkQueue& VulkanContext::getGraphicsQueue() const {
    return m_graphicsQueue;
}

const VkQueue& VulkanContext::getPresentQueue() const {
    return m_presentQueue;
}

void VulkanContext::m_pickPhysicalDevice(const VkSurfaceKHR &vkSurface) {
    fmt::println("Picking a suitable device");

    uint32_t deviceCount = 0;
    VK_CHECK("Failed to enumerate physical devices", vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, nullptr));
    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    VK_CHECK("Failed to enumerate physical devices",
             vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, devices.data()));

    for (const VkPhysicalDevice& device: devices) {
        PhysicalDevice physicalDevice(device, vkSurface);
        if (!physicalDevice.isSuitable()) {
            continue;
        }

        m_physicalDevice = std::make_unique<PhysicalDevice>(physicalDevice);
        break;
    }

    if (m_physicalDevice == nullptr) {
        throw std::runtime_error("Failed to find a suitable GPU");
    }
}

void VulkanContext::m_createLogicalDevice() {
    const QueueFamilyIndices &indices = m_physicalDevice->getQueueFamilyIndices();
    const std::vector requiredVKExtensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        "VK_KHR_portability_subset"
    };

    // Queues
    const float queuePriority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (const uint32_t index: indices.getUnique()) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = index;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Features
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    // Device creation
    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = requiredVKExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = requiredVKExtensions.data();

    VK_CHECK("Failed to create vk logical device",
             vkCreateDevice(m_physicalDevice->getUnderlying(), &deviceCreateInfo, nullptr, &m_device));

    // We now get the newly created queues
    vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
}

void VulkanContext::m_createCommandPool() {
    const QueueFamilyIndices& indices = m_physicalDevice->getQueueFamilyIndices();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = indices.graphicsFamily.value();

    VK_CHECK("failed to create command pool!",
             vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool));
}
