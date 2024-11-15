#pragma once

#include <SDL_video.h>
#include <vulkan/vulkan.h>

#include <memory>
#include <optional>
#include <set>
#include <vector>

#include "Buffer.h"
#include "objects/Triangle.h"

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

class VK {
   public:
    explicit VK(SDL_Window* window);
    void run();

   private:
    SDL_Window* m_window = nullptr;

    VkInstance m_instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;

    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;
    VkFormat m_swapChainImageFormat{};
    VkExtent2D m_swapChainExtent{};
    std::vector<VkFramebuffer> m_framebuffers;

    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_commandBuffers;
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    uint32_t m_currentFrame = 0;

    Triangle m_triangle;
    std::unique_ptr<Buffer> m_vertexBuffer;
    std::unique_ptr<Buffer> m_indexBuffer;

    std::vector<std::unique_ptr<Buffer>> m_uniformBuffers;
    std::vector<void*> m_uniformBuffersMapped;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_descriptorSets;

    void m_mainLoop();
    void m_drawFrame();
    void m_updateUniformBuffer(uint32_t imageIndex);

    // VK stuff
    [[nodiscard]]
    bool m_setupVVL(const std::vector<const char*>& requestedLayers) const;

    void m_createVKInstance();
    void m_createSurface();
    QueueFamilyIndices m_findQueueFamilies(VkPhysicalDevice device) const;
    bool m_checkDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails m_querySwapChainSupport(VkPhysicalDevice device);
    bool m_isDeviceSuitable(VkPhysicalDevice device);
    void m_pickPhysicalDevice();
    void m_createLogicalDevice();

    VkSurfaceFormatKHR m_chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR m_chooseSurfacePresentMode(const std::vector<VkPresentModeKHR>& availableModes);
    [[nodiscard]]
    VkExtent2D m_chooseSurfaceExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

    void m_createSwapChain();
    void m_destroySwapChain() const;
    void m_recreateSwapChain();
    void m_createImageViews();

    void m_createRenderPass();
    void m_createDescriptorSetLayout();
    void m_createGraphicsPipeline();
    void m_createFramebuffers();

    void m_createCommandPool();
    void m_createCommandBuffers();
    void m_createSyncObjects();

    void m_createVertexBuffer();
    void m_createIndexBuffer();
    void m_createUniformBuffers();
    void m_createDescriptorPool();
    void m_createDescriptorSets();

    void m_recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) const;

    void m_initVulkan();
    void m_destroyVulkan() const;
};
