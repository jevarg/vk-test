#pragma once

#include <optional>
#include <vector>
#include <set>
#include <SDL_video.h>
#include <vulkan/vulkan.h>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    [[nodiscard]]
    bool isValid() const {
        return graphicsFamily.has_value() &&
               presentFamily.has_value();
    }

    [[nodiscard]]
    std::set<uint32_t> getUnique() const {
        return {
            graphicsFamily.value(),
            presentFamily.value()
        };
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VK {
public:
    void run(SDL_Window *window);

private:
    VkInstance m_vkInstance = VK_NULL_HANDLE;
    VkSurfaceKHR m_vkSurface = VK_NULL_HANDLE;
    VkPhysicalDevice m_vkPhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_vkDevice = VK_NULL_HANDLE;
    VkPipelineLayout m_vkPipelineLayout = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;

    VkSwapchainKHR m_vkSwapChain = VK_NULL_HANDLE;
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


    // VK stuff
    [[nodiscard]]
    bool m_setupVVL(const std::vector<const char *> &requestedLayers) const;

    void m_createVKInstance(SDL_Window *window);
    void m_createSurface(SDL_Window *window);
    QueueFamilyIndices m_findQueueFamilies(VkPhysicalDevice device) const;
    bool m_checkDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails m_querySwapChainSupport(VkPhysicalDevice device);
    bool m_isDeviceSuitable(VkPhysicalDevice device);
    void m_pickPhysicalDevice();
    void m_createLogicalDevice();

    VkSurfaceFormatKHR m_chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR m_chooseSurfacePresentMode(const std::vector<VkPresentModeKHR>& availableModes);
    VkExtent2D m_chooseSurfaceExtent(const VkSurfaceCapabilitiesKHR &capabilities, SDL_Window *window);
    void m_createSwapChain(SDL_Window *window);
    void m_createImageViews();

    void m_createRenderPass();
    void m_createGraphicsPipeline();
    void m_createFramebuffers();

    void m_createCommandPool();
    void m_createCommandBuffers();
    void m_createSyncObjects();

    void m_initVulkan(SDL_Window *window);
    void m_destroyVulkan();

    void m_recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void m_mainLoop();
    void m_drawFrame();
};
