#include "VK.h"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_vulkan.h>
#include <fmt/base.h>
#include <fmt/format.h>
#include <vulkan/vk_enum_string_helper.h>

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <set>
#include <stdexcept>
#include <thread>

#include "gpu_resources/Shader.h"
#include "types/UniformBufferObject.h"
#include "types/Vertex.h"
#include "vkutil.h"

constexpr uint32_t maxInflightFrames = 2;

const std::vector requiredVKExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

VK::VK(SDL_Window* window) {
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

        m_drawFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    vkDeviceWaitIdle(m_vkContext.device);
}

void VK::m_drawFrame() {
    vkWaitForFences(m_vkContext.device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult res = vkAcquireNextImageKHR(m_vkContext.device, m_swapChain, UINT64_MAX,
                                         m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
        m_recreateSwapChain();
        return;
    }

    VK_CHECK("failed to acquire next image!", res);

    vkResetFences(m_vkContext.device, 1, &m_inFlightFences[m_currentFrame]);

    vkResetCommandBuffer(m_commandBuffers[m_currentFrame], 0);
    m_recordCommandBuffer(m_commandBuffers[m_currentFrame], imageIndex);
    m_updateUniformBuffer(m_currentFrame);

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
             vkQueueSubmit(m_vkContext.graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]));

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    const VkSwapchainKHR swapChains[] = { m_swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    res = vkQueuePresentKHR(m_presentQueue, &presentInfo);
    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
        m_recreateSwapChain();
    } else {
        VK_CHECK("failed to present queue!", res);
    }

    m_currentFrame = m_currentFrame % maxInflightFrames;
}

void VK::m_updateUniformBuffer(const uint32_t imageIndex) const {
    static auto startTime = std::chrono::high_resolution_clock::now();

    const auto currentTime = std::chrono::high_resolution_clock::now();
    const float time = std::chrono::duration<float>(currentTime - startTime).count();

    const glm::mat4 mat = rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    UniformBufferObject ubo{};
    ubo.model = rotate(mat, time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ubo.view = lookAt(glm::vec3(5.0f, 5.0f, 2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    const float aspectRatio =
        static_cast<float>(m_swapChainExtent.width) / static_cast<float>(m_swapChainExtent.height);
    ubo.projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 10.0f);
    ubo.projection[1][1] *= -1;  // inverting y because vulkan != gl

    memcpy(m_uniformBuffersMapped[imageIndex], &ubo, sizeof(UniformBufferObject));
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

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
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

QueueFamilyIndices VK::m_findQueueFamilies(VkPhysicalDevice device) const {
    QueueFamilyIndices indices{};

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    if (queueFamilyCount == 0) {
        throw std::runtime_error("Failed to get queue family count");
    }

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    uint32_t i = 0;
    for (const VkQueueFamilyProperties& queueFamily : queueFamilies) {
        if (indices.isValid()) {
            break;  // We have found every required indices
        }

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i++;
            continue;
        }

        VkBool32 isSupported;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &isSupported);
        if (isSupported) {
            indices.presentFamily = i++;
            continue;
        }

        ++i;
    }

    return indices;
}

bool VK::m_checkDeviceExtensionSupport(const VkPhysicalDevice device) {
    uint32_t extensionCount = 0;
    VK_CHECK("Failed to count device extensions",
             vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr));

    std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
    VK_CHECK("Failed to enumerate device extensions",
             vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, supportedExtensions.data()));

    for (const char* requiredExtension : requiredVKExtensions) {
        if (std::ranges::find_if(supportedExtensions.begin(), supportedExtensions.end(),
                                 [&requiredExtension](const VkExtensionProperties& props) {
                                     return strcmp(requiredExtension, props.extensionName) == 0;
                                 }) == supportedExtensions.end()) {
            return false;
        }
    }

    return true;
}

bool VK::m_isDeviceSuitable(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);

    // Keeping it simple for now.
    // Later on, we can implement a simple device score system
    // e.g.:
    // https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/03_Physical_devices_and_queue_families.html#_base_device_suitability_checks
    bool isSuitable = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && features.geometryShader &&
                      features.samplerAnisotropy && m_checkDeviceExtensionSupport(device);

    if (isSuitable) {
        const QueueFamilyIndices queueFamilyIndices = m_findQueueFamilies(device);
        const SwapChainSupportDetails swapChainSupport = m_querySwapChainSupport(device);

        bool swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();

        isSuitable = queueFamilyIndices.isValid() && swapChainAdequate;
    }

    fmt::println("{}: {}", properties.deviceName, isSuitable ? "OK" : "KO");
    return isSuitable;
}

void VK::m_pickPhysicalDevice() {
    fmt::println("Picking a suitable device");

    uint32_t deviceCount = 0;
    VK_CHECK("Failed to enumerate physical devices", vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr));

    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    VK_CHECK("Failed to enumerate physical devices",
             vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data()));

    for (VkPhysicalDevice device : devices) {
        if (!m_isDeviceSuitable(device)) {
            continue;
        }

        m_vkContext.physicalDevice = device;
        break;
    }

    if (m_vkContext.physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find a suitable GPU");
    }
}

void VK::m_createLogicalDevice() {
    fmt::println("Creating logical device");

    const QueueFamilyIndices indices = m_findQueueFamilies(m_vkContext.physicalDevice);
    if (!indices.isValid()) {
        throw std::runtime_error("Queue family indices is not valid (should not happen!)");
    }

    // Queues
    float queuePriority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (const uint32_t index : indices.getUnique()) {
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
             vkCreateDevice(m_vkContext.physicalDevice, &deviceCreateInfo, nullptr, &m_vkContext.device));

    // We now get the newly created queues
    vkGetDeviceQueue(m_vkContext.device, indices.graphicsFamily.value(), 0, &m_vkContext.graphicsQueue);
    vkGetDeviceQueue(m_vkContext.device, indices.presentFamily.value(), 0, &m_presentQueue);
}

SwapChainSupportDetails VK::m_querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    // Capabilities
    VK_CHECK("Failed to get surface capabilities",
             vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities));

    // Pixel formats
    uint32_t formatCount = 0;
    VK_CHECK("Failed to count surface pixel formats",
             vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr));
    if (formatCount) {
        details.formats.resize(formatCount);
        VK_CHECK("Failed to get surface pixel formats",
                 vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data()));
    }

    // Present modes
    uint32_t presentModeCount = 0;
    VK_CHECK("Failed to count surface present modes",
             vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr));
    if (presentModeCount) {
        details.presentModes.resize(presentModeCount);
        VK_CHECK("Failed to get surface present modes",
                 vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount,
                                                           details.presentModes.data()));
    }

    return details;
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
    const SwapChainSupportDetails swapChainDetails = m_querySwapChainSupport(m_vkContext.physicalDevice);
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
    const QueueFamilyIndices indices = m_findQueueFamilies(m_vkContext.physicalDevice);
    const uint32_t familyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily.value() == indices.presentFamily.value()) {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = familyIndices;
    }

    VK_CHECK("Failed to create vk swap chain",
             vkCreateSwapchainKHR(m_vkContext.device, &createInfo, nullptr, &m_swapChain));

    // Fetching swapChain images
    uint32_t imageCount = 0;
    VK_CHECK("Failed to count swap chain images",
             vkGetSwapchainImagesKHR(m_vkContext.device, m_swapChain, &imageCount, nullptr));

    m_swapChainImages.resize(imageCount);

    VK_CHECK("Failed to get swap chain images",
             vkGetSwapchainImagesKHR(m_vkContext.device, m_swapChain, &imageCount, m_swapChainImages.data()));

    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainExtent = extent;
}

void VK::m_destroySwapChain() const {
    for (const VkFramebuffer& framebuffer : m_framebuffers) {
        vkDestroyFramebuffer(m_vkContext.device, framebuffer, nullptr);
    }

    for (const VkImageView& imageView : m_swapChainImageViews) {
        vkDestroyImageView(m_vkContext.device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_vkContext.device, m_swapChain, nullptr);
}

void VK::m_recreateSwapChain() {
    fmt::println("Recreating swap chain");

    vkDeviceWaitIdle(m_vkContext.device);

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
                 vkCreateImageView(m_vkContext.device, &createInfo, nullptr, &m_swapChainImageViews[i]));
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
             vkCreateRenderPass(m_vkContext.device, &renderPassInfo, nullptr, &m_renderPass));
}

void VK::m_createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    const std::array bindings = {
        uboLayoutBinding,
        samplerLayoutBinding,
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = bindings.size();
    layoutInfo.pBindings = bindings.data();

    VK_CHECK("failed to create descriptor set layouts",
             vkCreateDescriptorSetLayout(m_vkContext.device, &layoutInfo, nullptr, &m_descriptorSetLayout));
}

void VK::m_createGraphicsPipeline() {
    const Shader vertShader(m_vkContext, "./shaders/tri.vert", Shader::Vertex);
    const Shader fragShader(m_vkContext, "./shaders/tri.frag", Shader::Fragment);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShader.getModule();
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShader.getModule();
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {
        vertShaderStageInfo,
        fragShaderStageInfo,
    };

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

    // const std::vector dynamicStates = {
    //     VK_DYNAMIC_STATE_VIEWPORT,
    //     VK_DYNAMIC_STATE_SCISSOR,
    // };

    // VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
    // dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    // dynamicStateInfo.dynamicStateCount = dynamicStates.size();
    // dynamicStateInfo.pDynamicStates = dynamicStates.data();

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

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;

    VK_CHECK("Failed to create pipeline layout!",
             vkCreatePipelineLayout(m_vkContext.device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vtxInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &depthStencil;
    // pipelineInfo.pDynamicState = &dynamicStateInfo;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;

    VK_CHECK(
        "failed to create graphics pipeline!",
        vkCreateGraphicsPipelines(m_vkContext.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline));
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
                 vkCreateFramebuffer(m_vkContext.device, &framebufferInfo, nullptr, &m_framebuffers[i]));
    }
}

void VK::m_createCommandPool() {
    const QueueFamilyIndices& indices = m_findQueueFamilies(m_vkContext.physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = indices.graphicsFamily.value();

    VK_CHECK("failed to create command pool!",
             vkCreateCommandPool(m_vkContext.device, &poolInfo, nullptr, &m_vkContext.commandPool));
}

void VK::m_createCommandBuffers() {
    m_commandBuffers.resize(maxInflightFrames);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_vkContext.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = m_commandBuffers.size();

    VK_CHECK("failed to create command buffers!",
             vkAllocateCommandBuffers(m_vkContext.device, &allocInfo, m_commandBuffers.data()));
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

    for (int i = 0; i < maxInflightFrames; ++i) {
        VK_CHECK("failed to create semaphore",
                 vkCreateSemaphore(m_vkContext.device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]));

        VK_CHECK("failed to create semaphore",
                 vkCreateSemaphore(m_vkContext.device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]));

        VK_CHECK("failed to create fence",
                 vkCreateFence(m_vkContext.device, &fenceInfo, nullptr, &m_inFlightFences[i]));
    }
}

void VK::m_createDepthResources() {
    VkExtent3D extent{};
    extent.width = m_swapChainExtent.width;
    extent.height = m_swapChainExtent.height;
    extent.depth = 1;

    m_depthImage = std::make_unique<DepthImage>(m_vkContext, extent);
    m_depthImage->transitionLayout(m_vkContext.commandPool, m_vkContext.graphicsQueue,
                                   VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void VK::m_createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    m_uniformBuffers.resize(maxInflightFrames);
    m_uniformBuffersMapped.resize(maxInflightFrames);

    for (int i = 0; i < maxInflightFrames; ++i) {
        m_uniformBuffers[i] =
            std::make_unique<Buffer>(m_vkContext, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkMapMemory(m_vkContext.device, m_uniformBuffers[i]->getMemory(), 0, m_uniformBuffers[i]->getSize(), 0,
                    &m_uniformBuffersMapped[i]);
    }
}

void VK::m_createDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = maxInflightFrames;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = maxInflightFrames;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = maxInflightFrames;

    VK_CHECK("failed to create descriptor pool",
             vkCreateDescriptorPool(m_vkContext.device, &poolInfo, nullptr, &m_descriptorPool));
}

void VK::m_createDescriptorSets() {
    const std::vector layouts(maxInflightFrames, m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = layouts.size();
    allocInfo.pSetLayouts = layouts.data();

    m_descriptorSets.resize(maxInflightFrames);
    VK_CHECK("failed to allocate descriptor sets",
             vkAllocateDescriptorSets(m_vkContext.device, &allocInfo, m_descriptorSets.data()));

    for (int i = 0; i < maxInflightFrames; ++i) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_uniformBuffers[i]->buffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.sampler = m_sampler;
        imageInfo.imageLayout = m_model->getTexture().getImage().getLayout();
        imageInfo.imageView = m_model->getTexture().getImage().getImageView();

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(m_vkContext.device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    }
}

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
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

    const std::array buffers = { m_model->getVertexBuffer().buffer() };
    constexpr std::array<VkDeviceSize, buffers.size()> offsets = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, buffers.size(), buffers.data(), offsets.data());
    vkCmdBindIndexBuffer(commandBuffer, m_model->getIndexBuffer().buffer(), 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1,
                            &m_descriptorSets[m_currentFrame], 0, nullptr);
    vkCmdDrawIndexed(commandBuffer, m_model->getIndices().size(), 1, 0, 0, 0);
    vkCmdEndRenderPass(commandBuffer);

    VK_CHECK("failed to record command buffer!", vkEndCommandBuffer(commandBuffer));
}

void VK::m_initVulkan() {
    fmt::println("Initializing vk");

    m_createVKInstance();
    m_createSurface();
    m_pickPhysicalDevice();
    m_createLogicalDevice();
    m_createSwapChain();
    m_createImageViews();
    m_createRenderPass();
    m_createUniformBuffers();
    m_createCommandPool();
    m_createDepthResources();

    m_model = std::make_unique<Model>(m_vkContext, "./assets/jg.obj", "./assets/jg.png");

    m_createSampler();

    m_createDescriptorSetLayout();
    m_createDescriptorPool();
    m_createDescriptorSets();
    m_createGraphicsPipeline();
    m_createFramebuffers();

    // m_createVertexBuffer();
    // m_createIndexBuffer();
    m_createCommandBuffers();
    m_createSyncObjects();

    fmt::println("Good to go :)");
}

void VK::m_destroyVulkan() const {
    m_destroySwapChain();

    for (int i = 0; i < maxInflightFrames; ++i) {
        m_uniformBuffers[i]->destroy();
    }

    vkDestroyDescriptorPool(m_vkContext.device, m_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(m_vkContext.device, m_descriptorSetLayout, nullptr);

    m_depthImage->destroy();

    vkDestroySampler(m_vkContext.device, m_sampler, nullptr);
    m_model->destroy();

    vkDestroyPipeline(m_vkContext.device, m_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_vkContext.device, m_pipelineLayout, nullptr);
    vkDestroyRenderPass(m_vkContext.device, m_renderPass, nullptr);

    for (int i = 0; i < maxInflightFrames; ++i) {
        vkDestroySemaphore(m_vkContext.device, m_imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(m_vkContext.device, m_renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(m_vkContext.device, m_inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(m_vkContext.device, m_vkContext.commandPool, nullptr);
    vkDestroyDevice(m_vkContext.device, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);

    fmt::println("Destroyed vk resources");
}

void VK::m_createSampler() {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_vkContext.physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VK_CHECK("failed to create sampler", vkCreateSampler(m_vkContext.device, &samplerInfo, nullptr, &m_sampler));
}
