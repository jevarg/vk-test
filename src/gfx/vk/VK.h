#pragma once

#include <SDL_video.h>
#include <vulkan/vulkan_core.h>

#include <memory>
#include <vector>

#include "gfx/Camera.h"
#include "gpu_resources/DepthImage.h"
#include "objects/Model.h"

class VK {
   public:
    explicit VK(SDL_Window* window);
    void run();

   private:
    SDL_Window* m_window = nullptr;

    VkInstance m_instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;

    struct Pipeline {
        VkPipeline scene;
        VkPipeline skybox;
    } m_pipelines;

    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;
    VkFormat m_swapChainImageFormat{};
    VkExtent2D m_swapChainExtent{};
    std::vector<VkFramebuffer> m_framebuffers;

    std::vector<VkCommandBuffer> m_commandBuffers;
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    uint32_t m_currentFrame = 0;

    std::unique_ptr<DepthImage> m_depthImage;

    std::vector<Texture> m_textures;
    std::vector<Model> m_models;

    std::unique_ptr<Camera> m_camera;

    VkDescriptorSetLayout m_sceneDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_textureDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

    void m_mainLoop();
    void m_drawFrame();

    // VK stuff
    [[nodiscard]]
    bool m_setupVVL(const std::vector<const char*>& requestedLayers) const;

    void m_createVKInstance();
    void m_createSurface();

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

    void m_createCommandBuffers();
    void m_createSyncObjects();

    void m_createDepthResources();

    // void m_createUniformBuffers();
    void m_createDescriptorPool();
    // void m_createDescriptorSets();

    void m_recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) const;
    void m_drawModels(VkCommandBuffer commandBuffer) const;

    void m_initVulkan();
    void m_destroyVulkan() const;
};
