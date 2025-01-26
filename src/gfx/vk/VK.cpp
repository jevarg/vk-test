#include "VK.h"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_vulkan.h>
#include <fmt/base.h>
#include <fmt/format.h>
#include <vulkan/vk_enum_string_helper.h>

#include <algorithm>
#include <glm/glm.hpp>
#include <stdexcept>
#include <thread>

#include "gpu_resources/Shader.h"
#include "input/Keyboard.h"
#include "input/Mouse.h"
#include "objects/prefabs/Cube.h"
#include "objects/prefabs/Plane.h"
#include "types/Vertex.h"
#include "vkutil.h"

constexpr uint32_t maxInflightFrames = 1;

const std::vector requiredVKExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

VK::VK(SDL_Window* window) {
    Keyboard::init();
    m_window = window;
}

void VK::run() {
    m_initVulkan();
    m_mainLoop();
    m_destroyVulkan();
}

void VK::m_mainLoop() {
    bool shouldClose = true;
    while (shouldClose) {
        SDL_Event evt;
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT || (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_ESCAPE)) {
                shouldClose = false;
                break;
            }
        }

        Keyboard::update();
        Mouse::update();

        m_camera->update(0);

        m_drawFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    vkDeviceWaitIdle(VulkanContext::get().getDevice());
}

void VK::m_drawFrame() {
    const VulkanContext& vkContext = VulkanContext::get();
    vkWaitForFences(vkContext.getDevice(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult res = vkAcquireNextImageKHR(vkContext.getDevice(), m_swapChain, UINT64_MAX,
                                         m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
        m_recreateSwapChain();
        return;
    }

    VK_CHECK("failed to acquire next image!", res);

    vkResetFences(vkContext.getDevice(), 1, &m_inFlightFences[m_currentFrame]);

    vkResetCommandBuffer(m_commandBuffers[m_currentFrame], 0);
    m_recordCommandBuffer(m_commandBuffers[m_currentFrame], imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    const VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
    const VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers[m_currentFrame];

    const VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VK_CHECK("failed to submit draw command buffer!",
             vkQueueSubmit(vkContext.getGraphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]));

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    const VkSwapchainKHR swapChains[] = { m_swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    res = vkQueuePresentKHR(vkContext.getPresentQueue(), &presentInfo);
    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
        m_recreateSwapChain();
    } else {
        VK_CHECK("failed to present queue!", res);
    }

    m_currentFrame = m_currentFrame % maxInflightFrames;
}

bool VK::m_setupVVL(const std::vector<const char*>& requestedLayers) const {
#ifdef NDEBUG
    return false;
#endif
    fmt::println("Setting up VVL");

    uint32_t layersCount = 0;
    VK_CHECK("Failed to count layer properties", vkEnumerateInstanceLayerProperties(&layersCount, nullptr));

    std::vector<VkLayerProperties> availableLayers(layersCount);
    VK_CHECK("Failed to enumerate layer properties",
             vkEnumerateInstanceLayerProperties(&layersCount, availableLayers.data()));

    for (const auto& requestedLayer : requestedLayers) {
        if (std::ranges::find_if(availableLayers, [&requestedLayer](const auto& availableLayer) {
                return strcmp(availableLayer.layerName, requestedLayer) == 0;
            }) == availableLayers.end()) {
            throw std::runtime_error(fmt::format("Failed to find requested layer '{}'", requestedLayer));
        }
    }

    return true;
}

void VK::m_createVKInstance() {
    fmt::println("Creating vk instance");
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "VKTest";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "MEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    uint32_t extCount;
    bool success = SDL_Vulkan_GetInstanceExtensions(m_window, &extCount, nullptr);
    if (!success) {
        throw std::runtime_error("Failed to get instance extensions count");
    }

    std::vector<const char*> instanceExtensions(extCount);
    success = SDL_Vulkan_GetInstanceExtensions(m_window, &extCount, instanceExtensions.data());
    if (!success) {
        throw std::runtime_error("Failed to get instance extensions names");
    }

    fmt::println("SDL instance exts");
    for (auto ext : instanceExtensions) {
        fmt::println("  {}", ext);
    }

    instanceExtensions.push_back("VK_KHR_portability_enumeration");
    instanceExtensions.push_back("VK_KHR_get_physical_device_properties2");

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = instanceExtensions.size();
    createInfo.ppEnabledExtensionNames = instanceExtensions.data();

    // Validation layers setup
    const std::vector layers = {
        "VK_LAYER_KHRONOS_validation",
    };

    if (m_setupVVL(layers)) {
        createInfo.enabledLayerCount = layers.size();
        createInfo.ppEnabledLayerNames = layers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    VK_CHECK("Failed to create vk instance", vkCreateInstance(&createInfo, nullptr, &m_instance));
}

void VK::m_createSurface() {
    fmt::println("Creating vk surface");

    if (!SDL_Vulkan_CreateSurface(m_window, m_instance, &m_surface)) {
        throw std::runtime_error(fmt::format("Unable to create vk surface: {}", SDL_GetError()));
    }
}

VkSurfaceFormatKHR VK::m_chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR VK::m_chooseSurfacePresentMode(const std::vector<VkPresentModeKHR>& availableModes) {
    for (const auto& availableMode : availableModes) {
        if (availableMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            return availableMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;  // VSync
}

VkExtent2D VK::m_chooseSurfaceExtent(const VkSurfaceCapabilitiesKHR& capabilities) const {
    int width = 0;
    int height = 0;

    SDL_Vulkan_GetDrawableSize(m_window, &width, &height);
    VkExtent2D extent{};

    extent.width = std::ranges::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width,
                                      capabilities.maxImageExtent.width);
    extent.height = std::ranges::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height,
                                       capabilities.maxImageExtent.height);

    return extent;
}

void VK::m_createSwapChain() {
    const VulkanContext& vkContext = VulkanContext::get();
    const SwapChainSupportDetails& swapChainDetails = vkContext.getPhysicalDevice().getSwapChainSupportDetails();
    const VkSurfaceFormatKHR surfaceFormat = m_chooseSurfaceFormat(swapChainDetails.formats);
    const VkPresentModeKHR presentMode = m_chooseSurfacePresentMode(swapChainDetails.presentModes);
    const VkExtent2D extent = m_chooseSurfaceExtent(swapChainDetails.capabilities);

    // Basic SwapChain settings
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = swapChainDetails.capabilities.minImageCount + 1;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.presentMode = presentMode;
    createInfo.preTransform = swapChainDetails.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    // QueueFamilies handling
    const QueueFamilyIndices& indices = vkContext.getPhysicalDevice().getQueueFamilyIndices();
    const uint32_t familyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily.value() == indices.presentFamily.value()) {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = familyIndices;
    }

    VK_CHECK("Failed to create vk swap chain",
             vkCreateSwapchainKHR(vkContext.getDevice(), &createInfo, nullptr, &m_swapChain));

    // Fetching swapChain images
    uint32_t imageCount = 0;
    VK_CHECK("Failed to count swap chain images",
             vkGetSwapchainImagesKHR(vkContext.getDevice(), m_swapChain, &imageCount, nullptr));

    m_swapChainImages.resize(imageCount);

    VK_CHECK("Failed to get swap chain images",
             vkGetSwapchainImagesKHR(vkContext.getDevice(), m_swapChain, &imageCount, m_swapChainImages.data()));

    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainExtent = extent;
}

void VK::m_destroySwapChain() const {
    const VkDevice& device = VulkanContext::get().getDevice();

    for (const VkFramebuffer& framebuffer : m_framebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    for (const VkImageView& imageView : m_swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, m_swapChain, nullptr);
}

void VK::m_recreateSwapChain() {
    fmt::println("Recreating swap chain");

    vkDeviceWaitIdle(VulkanContext::get().getDevice());

    m_destroySwapChain();

    m_createSwapChain();
    m_createImageViews();

    m_depthImage->destroy();
    m_createDepthResources();

    m_createFramebuffers();
}

void VK::m_createImageViews() {
    m_swapChainImageViews.resize(m_swapChainImages.size());

    for (size_t i = 0; i < m_swapChainImages.size(); ++i) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_swapChainImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VK_CHECK("Failed to create image view",
                 vkCreateImageView(VulkanContext::get().getDevice(), &createInfo, nullptr, &m_swapChainImageViews[i]));
    }
}

void VK::m_createRenderPass() {
    // Color attachment
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth attachment
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = VK_FORMAT_D32_SFLOAT;  // TODO: findDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    const std::array attachments = { colorAttachment, depthAttachment };

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VK_CHECK("Failed to create render pass!",
             vkCreateRenderPass(VulkanContext::get().getDevice(), &renderPassInfo, nullptr, &m_renderPass));
}

void VK::m_createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding sceneLayoutBinding{};
    sceneLayoutBinding.binding = 0;
    sceneLayoutBinding.descriptorCount = 1;
    sceneLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    sceneLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &sceneLayoutBinding;

    VK_CHECK("failed to create scene descriptor set layout",
             vkCreateDescriptorSetLayout(VulkanContext::get().getDevice(), &layoutInfo, nullptr,
                                         &m_sceneDescriptorSetLayout));

    VkDescriptorSetLayoutBinding textureLayoutBinding{};
    textureLayoutBinding.binding = 0;
    textureLayoutBinding.descriptorCount = 1;
    textureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    layoutInfo.pBindings = &textureLayoutBinding;
    VK_CHECK("failed to create texture descriptor set layout",
             vkCreateDescriptorSetLayout(VulkanContext::get().getDevice(), &layoutInfo, nullptr,
                                         &m_textureDescriptorSetLayout));
}

void VK::m_createGraphicsPipeline() {
    const VulkanContext& vkContext = VulkanContext::get();

    VkVertexInputBindingDescription bindingDescription = Vertex::getBindingDescription();
    std::array attributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vtxInputInfo{};
    vtxInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vtxInputInfo.vertexBindingDescriptionCount = 1;
    vtxInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vtxInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    vtxInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport viewport{};
    viewport.width = static_cast<float>(m_swapChainExtent.width);
    viewport.height = static_cast<float>(m_swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = m_swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_SUBTRACT;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

    VkPushConstantRange pushConstant{};
    pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstant.offset = 0;
    pushConstant.size = sizeof(glm::mat4);

    const VkDescriptorSetLayout layouts[]{ m_sceneDescriptorSetLayout, m_textureDescriptorSetLayout };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 2;
    pipelineLayoutInfo.pSetLayouts = layouts;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstant;

    VK_CHECK("Failed to create pipeline layout!",
             vkCreatePipelineLayout(vkContext.getDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout));

    m_pipelines.scene = std::make_unique<Pipeline>(
        Pipeline::Type::Graphics, "./shaders/tri.vert", "./shaders/tri.frag", vtxInputInfo, inputAssembly,
        viewportState, rasterizer, multisampling, colorBlending, depthStencil, m_pipelineLayout, m_renderPass);

    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthTestEnable = VK_FALSE;

    rasterizer.cullMode = VK_CULL_MODE_NONE;
    // rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    m_pipelines.skybox = std::make_unique<Pipeline>(
        Pipeline::Type::Graphics, "./shaders/skybox.vert", "./shaders/skybox.frag", vtxInputInfo, inputAssembly,
        viewportState, rasterizer, multisampling, colorBlending, depthStencil, m_pipelineLayout, m_renderPass);
}

void VK::m_createFramebuffers() {
    m_framebuffers.resize(m_swapChainImageViews.size());
    for (int i = 0; i < m_swapChainImageViews.size(); ++i) {
        const std::array attachments = { m_swapChainImageViews[i], m_depthImage->getImageView() };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_swapChainExtent.width;
        framebufferInfo.height = m_swapChainExtent.height;
        framebufferInfo.layers = 1;

        VK_CHECK(fmt::format("unable to create framebuffer {}", i).c_str(),
                 vkCreateFramebuffer(VulkanContext::get().getDevice(), &framebufferInfo, nullptr, &m_framebuffers[i]));
    }
}

void VK::m_createCommandBuffers() {
    const VulkanContext& vkContext = VulkanContext::get();
    m_commandBuffers.resize(maxInflightFrames);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vkContext.getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = m_commandBuffers.size();

    VK_CHECK("failed to create command buffers!",
             vkAllocateCommandBuffers(vkContext.getDevice(), &allocInfo, m_commandBuffers.data()));
}

void VK::m_createSyncObjects() {
    m_imageAvailableSemaphores.resize(maxInflightFrames);
    m_renderFinishedSemaphores.resize(maxInflightFrames);
    m_inFlightFences.resize(maxInflightFrames);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    const VulkanContext& vkContext = VulkanContext::get();

    for (int i = 0; i < maxInflightFrames; ++i) {
        VK_CHECK("failed to create semaphore",
                 vkCreateSemaphore(vkContext.getDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]));

        VK_CHECK("failed to create semaphore",
                 vkCreateSemaphore(vkContext.getDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]));

        VK_CHECK("failed to create fence",
                 vkCreateFence(vkContext.getDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]));
    }
}

void VK::m_createDepthResources() {
    VkExtent3D extent{};
    extent.width = m_swapChainExtent.width;
    extent.height = m_swapChainExtent.height;
    extent.depth = 1;

    m_depthImage = std::make_unique<DepthImage>(extent);
    m_depthImage->transitionLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

// void VK::m_createUniformBuffers() {
//     VkDeviceSize bufferSize = sizeof(UniformBufferObject);
//
//     m_uniformBuffers.resize(maxInflightFrames);
//     m_uniformBuffersMapped.resize(maxInflightFrames);
//
//     for (int i = 0; i < maxInflightFrames; ++i) {
//         m_uniformBuffers[i] =
//             std::make_unique<Buffer>(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
//                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
//         vkMapMemory(VulkanContext::get().getDevice(), m_uniformBuffers[i]->getMemory(), 0,
//         m_uniformBuffers[i]->getSize(), 0,
//                     &m_uniformBuffersMapped[i]);
//     }
// }

void VK::m_createDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = 2;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 3;  // TODO: Do that

    VK_CHECK("failed to create descriptor pool",
             vkCreateDescriptorPool(VulkanContext::get().getDevice(), &poolInfo, nullptr, &m_descriptorPool));
}

// void VK::m_createDescriptorSets() {
//     const VulkanContext& vkContext = VulkanContext::get();
//     const std::vector layouts(maxInflightFrames, m_descriptorSetLayout);
//     VkDescriptorSetAllocateInfo allocInfo{};
//     allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//     allocInfo.descriptorPool = m_descriptorPool;
//     allocInfo.descriptorSetCount = layouts.size();
//     allocInfo.pSetLayouts = layouts.data();
//
//     m_descriptorSets.resize(maxInflightFrames);
//     VK_CHECK("failed to allocate descriptor sets",
//              vkAllocateDescriptorSets(vkContext.getDevice(), &allocInfo, m_descriptorSets.data()));

// for (int i = 0; i < maxInflightFrames; ++i) {
// VkDescriptorBufferInfo bufferInfo{};
// bufferInfo.buffer = m_uniformBuffers[i]->buffer();
// bufferInfo.offset = 0;
// bufferInfo.range = sizeof(UniformBufferObject);

// VkDescriptorImageInfo imageInfo{};
// imageInfo.sampler = m_sampler;
// // TODO: Deal with that
//
// const Texture& texture = m_textures.at(m_models[0].getTextureID());
// imageInfo.imageLayout = texture.getImage().getLayout();
// imageInfo.imageView = texture.getImage().getImageView();

// VkWriteDescriptorSet descriptorWrite{};
// descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
// descriptorWrite.dstSet = m_descriptorSets[i];
// descriptorWrite.dstBinding = 0;
// descriptorWrite.dstArrayElement = 0;
// descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
// descriptorWrite.descriptorCount = 1;
// descriptorWrite.pBufferInfo = &bufferInfo;

// descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
// descriptorWrites[1].dstSet = m_descriptorSets[i];
// descriptorWrites[1].dstBinding = 1;
// descriptorWrites[1].dstArrayElement = 0;
// descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
// descriptorWrites[1].descriptorCount = 1;
// descriptorWrites[1].pImageInfo = &imageInfo;

// vkUpdateDescriptorSets(vkContext.getDevice(), 1, &descriptorWrite, 0, nullptr);
// }
// }

void VK::m_recordCommandBuffer(VkCommandBuffer commandBuffer, const uint32_t imageIndex) const {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VK_CHECK("failed to begin recording command buffer", vkBeginCommandBuffer(commandBuffer, &beginInfo));

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_framebuffers[imageIndex];

    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_swapChainExtent;

    // clang-format off
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{ 0, 0, 0, 1.0f }};
    clearValues[1].depthStencil = { 1.0f, 0 };
    // clang-format on

    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    m_pipelines.skybox->bind(commandBuffer);

    const Texture& skyTex = m_textures[m_skybox->getTextureID()];
    const std::array descriptorSets{
        m_camera->getDescriptorSet(),
        skyTex.getDescriptorSet()
    };

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, descriptorSets.size(),
                            descriptorSets.data(), 0, nullptr);
    m_skybox->draw(commandBuffer, m_pipelineLayout);

    m_pipelines.scene->bind(commandBuffer);
    m_drawModels(commandBuffer);
    vkCmdEndRenderPass(commandBuffer);

    VK_CHECK("failed to record command buffer!", vkEndCommandBuffer(commandBuffer));
}

void VK::m_drawModels(VkCommandBuffer commandBuffer) const {
    for (const auto& model : m_models) {
        const Texture& texture = m_textures[model.getTextureID()];
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 1, 1,
                                &texture.getDescriptorSet(), 0, nullptr);

        model.draw(commandBuffer, m_pipelineLayout);
    }
}

void VK::m_initVulkan() {
    fmt::println("Initializing vk");

    m_createVKInstance();
    m_createSurface();

    VulkanContext::get().init(m_instance, m_surface);

    m_createSwapChain();
    m_createImageViews();
    m_createRenderPass();
    m_createDepthResources();
    m_createDescriptorSetLayout();
    m_createDescriptorPool();

    m_textures.emplace_back(std::vector{"./assets/viking_room.png"}, m_descriptorPool, m_textureDescriptorSetLayout);
    m_textures.emplace_back(std::vector{
        "./assets/skybox/hl1/right.bmp",
        "./assets/skybox/hl1/left.bmp",
        "./assets/skybox/hl1/top.bmp",
        "./assets/skybox/hl1/bottom.bmp",
        "./assets/skybox/hl1/back.bmp",
        "./assets/skybox/hl1/front.bmp",
    }, m_descriptorPool, m_textureDescriptorSetLayout);

    m_models.emplace_back("./assets/viking_room.obj", m_textures[0].getID());
    m_skybox = std::make_unique<Cube>(m_textures[1].getID());

    // m_createDescriptorSets();
    m_createGraphicsPipeline();
    m_createFramebuffers();

    m_createCommandBuffers();
    m_createSyncObjects();

    const float aspectRatio =
        static_cast<float>(m_swapChainExtent.width) / static_cast<float>(m_swapChainExtent.height);
    m_camera = std::make_unique<Camera>(aspectRatio, m_descriptorPool, m_sceneDescriptorSetLayout);
    m_camera->setPosition({ 0.0f, 0.0f, 5.0f });

    fmt::println("Good to go :)");
}

void VK::m_destroyVulkan() const {
    m_destroySwapChain();

    VulkanContext& vkContext = VulkanContext::get();

    m_camera->destroy();
    vkDestroyDescriptorPool(vkContext.getDevice(), m_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(vkContext.getDevice(), m_sceneDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(vkContext.getDevice(), m_textureDescriptorSetLayout, nullptr);

    m_depthImage->destroy();
    for (const auto& texture : m_textures) {
        texture.destroy();
    }

    m_skybox->destroy();
    for (const auto& model : m_models) {
        model.destroy();
    }

    m_pipelines.scene->destroy();
    m_pipelines.skybox->destroy();

    vkDestroyPipelineLayout(vkContext.getDevice(), m_pipelineLayout, nullptr);
    vkDestroyRenderPass(vkContext.getDevice(), m_renderPass, nullptr);

    for (int i = 0; i < maxInflightFrames; ++i) {
        vkDestroySemaphore(vkContext.getDevice(), m_imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(vkContext.getDevice(), m_renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(vkContext.getDevice(), m_inFlightFences[i], nullptr);
    }

    vkContext.destroy();
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);

    fmt::println("Destroyed vk resources");
}

// void VK::m_createSampler() {
//     const VulkanContext& vkContext = VulkanContext::get();
//
//     VkPhysicalDeviceProperties properties{};
//     vkGetPhysicalDeviceProperties(vkContext.getPhysicalDevice().getUnderlying(), &properties);
//
//     VkSamplerCreateInfo samplerInfo{};
//     samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
//     samplerInfo.magFilter = VK_FILTER_NEAREST;
//     samplerInfo.minFilter = VK_FILTER_NEAREST;
//
//     samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//     samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//     samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//
//     samplerInfo.anisotropyEnable = VK_TRUE;
//     samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
//     samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
//     samplerInfo.unnormalizedCoordinates = VK_FALSE;
//
//     samplerInfo.compareEnable = VK_FALSE;
//     samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
//
//     samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
//     samplerInfo.mipLodBias = 0.0f;
//     samplerInfo.minLod = 0.0f;
//     samplerInfo.maxLod = 0.0f;
//
//     VK_CHECK("failed to create sampler", vkCreateSampler(vkContext.getDevice(), &samplerInfo, nullptr, &m_sampler));
// }
