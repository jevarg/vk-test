#pragma once

#include <memory>

#include "../gpu_resources/PhysicalDevice.h"

class VulkanContext {
public:
    static VulkanContext& get();

    void init(const VkInstance& vkInstance, const VkSurfaceKHR& vkSurface);
    void destroy();

    bool isInitialized() const;

    const PhysicalDevice& getPhysicalDevice() const;
    const VkDevice& getDevice() const;

    const VkCommandPool& getCommandPool() const;
    const VkQueue& getGraphicsQueue() const;
    const VkQueue& getPresentQueue() const;

private:
    void m_pickPhysicalDevice(const VkSurfaceKHR& vkSurface);
    void m_createLogicalDevice();
    void m_createCommandPool();

    bool m_initialized = false;

    VkInstance m_vkInstance = VK_NULL_HANDLE;

    std::unique_ptr<PhysicalDevice> m_physicalDevice;
    VkPhysicalDevice m_physDev = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;

    VkCommandPool m_commandPool = VK_NULL_HANDLE;

    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
};
