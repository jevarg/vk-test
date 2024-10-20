#include "VK.h"

#include <algorithm>
#include <set>
#include <stdexcept>
#include <thread>
#include <fmt/base.h>
#include <fmt/format.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#include "Shader.h"

const std::vector requiredVKExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

void VK::run(SDL_Window *window) {
    m_initVulkan(window);

    bool shouldClose = true;
    while (shouldClose) {
        SDL_Event evt;
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT ||
                (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_ESCAPE)) {
                shouldClose = false;
                break;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    m_destroyVulkan();
}

void VK::m_mainLoop() {
}

bool VK::m_setupVVL(const std::vector<const char *> &requestedLayers) const {
#ifdef NDEBUG
    return false;
#endif
    fmt::println("Setting up VVL");

    uint32_t layersCount = 0;
    VkResult res = vkEnumerateInstanceLayerProperties(&layersCount, nullptr);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to count layer properties");
    }

    std::vector<VkLayerProperties> availableLayers(layersCount);
    res = vkEnumerateInstanceLayerProperties(&layersCount, availableLayers.data());
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to enumerate layer properties");
    }

    for (const auto &requestedLayer: requestedLayers) {
        if (std::ranges::find_if(availableLayers, [&requestedLayer](const auto &availableLayer) {
            return strcmp(availableLayer.layerName, requestedLayer) == 0;
        }) == availableLayers.end()) {
            throw std::runtime_error(fmt::format("Failed to find requested layer '{}'", requestedLayer));
        }
    }

    return true;
}

void VK::m_createVKInstance(SDL_Window *window) {
    fmt::println("Creating vk instance");
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "VKTest";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "MEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    uint32_t extCount;
    bool res = SDL_Vulkan_GetInstanceExtensions(window, &extCount, nullptr);
    if (!res) {
        throw std::runtime_error("Failed to get instance extensions count");
    }

    std::vector<const char *> instanceExtensions(extCount);
    res = SDL_Vulkan_GetInstanceExtensions(window, &extCount, instanceExtensions.data());
    if (!res) {
        throw std::runtime_error("Failed to get instance extensions names");
    }

    fmt::println("SDL instance exts");
    for (auto ext: instanceExtensions) {
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

    if (auto res = vkCreateInstance(&createInfo, nullptr, &m_vkInstance); res != VK_SUCCESS) {
        throw std::runtime_error(fmt::format("Failed to create vk instance ({})", string_VkResult(res)));
    }
}

void VK::m_createSurface(SDL_Window *window) {
    fmt::println("Creating vk surface");

    if (!SDL_Vulkan_CreateSurface(window, m_vkInstance, &m_vkSurface)) {
        throw std::runtime_error(fmt::format("Unable to create vk surface: {}", SDL_GetError()));
    }
}

QueueFamilyIndices VK::m_findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices{};

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    if (queueFamilyCount == 0) {
        throw std::runtime_error("Failed to get queue family count");
    }

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    uint32_t i = 0;
    for (const VkQueueFamilyProperties &queueFamily: queueFamilies) {
        if (indices.isValid()) {
            break; // We have found every required indices
        }

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i++;
            continue;
        }

        VkBool32 isSupported;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_vkSurface, &isSupported);
        if (isSupported) {
            indices.presentFamily = i++;
            continue;
        }

        ++i;
    }

    return indices;
}

bool VK::m_checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount = 0;
    VkResult res = vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to count device extensions");
    }

    std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
    res = vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, supportedExtensions.data());
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to enumerate device extensions");
    }

    for (const char *requiredExtension: requiredVKExtensions) {
        if (std::ranges::find_if(supportedExtensions.begin(), supportedExtensions.end(),
                                 [&requiredExtension](const VkExtensionProperties &props) {
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
    // e.g.: https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/03_Physical_devices_and_queue_families.html#_base_device_suitability_checks
    bool isSuitable = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
                      features.geometryShader &&
                      m_checkDeviceExtensionSupport(device);

    if (isSuitable) {
        const QueueFamilyIndices queueFamilyIndices = m_findQueueFamilies(device);
        const SwapChainSupportDetails swapChainSupport = m_querySwapChainSupport(device);

        bool swapChainAdequate = !swapChainSupport.formats.empty() &&
                                 !swapChainSupport.presentModes.empty();

        isSuitable = queueFamilyIndices.isValid() && swapChainAdequate;
    }

    fmt::println("{}: {}", properties.deviceName, isSuitable ? "OK" : "KO");
    return isSuitable;
}

void VK::m_pickPhysicalDevice() {
    fmt::println("Picking a suitable device");

    uint32_t deviceCount = 0;
    VkResult res = vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, nullptr);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to enumerate physical devices");
    }

    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    res = vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, devices.data());
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to enumerate physical devices");
    }

    for (VkPhysicalDevice device: devices) {
        if (!m_isDeviceSuitable(device)) {
            continue;
        }

        m_vkPhysicalDevice = device;
        break;
    }

    if (m_vkPhysicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find a suitable GPU");
    }
}

void VK::m_createLogicalDevice() {
    fmt::println("Creating logical device");

    const QueueFamilyIndices indices = m_findQueueFamilies(m_vkPhysicalDevice);
    if (!indices.isValid()) {
        throw std::runtime_error("Queue family indices is not valid (should not happen!)");
    }

    // Queues
    float queuePriority = 1.0f;
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

    // Device creation
    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = requiredVKExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = requiredVKExtensions.data();

    const VkResult res = vkCreateDevice(m_vkPhysicalDevice, &deviceCreateInfo, nullptr, &m_vkDevice);
    if (res != VK_SUCCESS) {
        throw std::runtime_error(fmt::format("Failed to create vk logical device ({})", string_VkResult(res)));
    }

    // We now get the newly created queues
    vkGetDeviceQueue(m_vkDevice, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_vkDevice, indices.presentFamily.value(), 0, &m_presentQueue);
}

SwapChainSupportDetails VK::m_querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    // Capabilities
    VkResult res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_vkSurface, &details.capabilities);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to get surface capabilities");
    }

    // Pixel formats
    uint32_t formatCount = 0;
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_vkSurface, &formatCount, nullptr);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to count surface pixel formats");
    }
    if (formatCount) {
        details.formats.resize(formatCount);
        res = vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_vkSurface, &formatCount,
                                                   details.formats.data());
        if (res != VK_SUCCESS) {
            throw std::runtime_error("Failed to get surface pixel formats");
        }
    }

    // Present modes
    uint32_t presentModeCount = 0;
    res = vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_vkSurface, &presentModeCount, nullptr);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to count surface present modes");
    }
    if (presentModeCount) {
        details.presentModes.resize(presentModeCount);
        res = vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_vkSurface, &presentModeCount,
                                                        details.presentModes.data());
        if (res != VK_SUCCESS) {
            throw std::runtime_error("Failed to get surface present modes");
        }
    }

    return details;
}

VkSurfaceFormatKHR VK::m_chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for (const auto &availableFormat: availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
            }
    }

    return availableFormats[0];
}

VkPresentModeKHR VK::m_chooseSurfacePresentMode(const std::vector<VkPresentModeKHR> &availableModes) {
    for (const auto &availableMode: availableModes) {
        if (availableMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            return availableMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR; // VSync
}

VkExtent2D VK::m_chooseSurfaceExtent(const VkSurfaceCapabilitiesKHR &capabilities, SDL_Window *window) {
    int width = 0;
    int height = 0;

    SDL_Vulkan_GetDrawableSize(window, &width, &height);
    VkExtent2D extent{};

    extent.width = std::ranges::clamp(
        static_cast<uint32_t>(width),
        capabilities.minImageExtent.width,
        capabilities.maxImageExtent.width
    );
    extent.height = std::ranges::clamp(
        static_cast<uint32_t>(height),
        capabilities.minImageExtent.height,
        capabilities.maxImageExtent.height
    );

    return extent;
}

void VK::m_createSwapChain(SDL_Window *window) {
    const SwapChainSupportDetails swapChainDetails = m_querySwapChainSupport(m_vkPhysicalDevice);
    const VkSurfaceFormatKHR surfaceFormat = m_chooseSurfaceFormat(swapChainDetails.formats);
    const VkPresentModeKHR presentMode = m_chooseSurfacePresentMode(swapChainDetails.presentModes);
    const VkExtent2D extent = m_chooseSurfaceExtent(swapChainDetails.capabilities, window);

    // Basic SwapChain settings
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_vkSurface;
    createInfo.minImageCount = swapChainDetails.capabilities.minImageCount + 1;;
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
    const QueueFamilyIndices indices = m_findQueueFamilies(m_vkPhysicalDevice);
    const uint32_t familyIndices[] = {
        indices.graphicsFamily.value(),
        indices.presentFamily.value()
    };

    if (indices.graphicsFamily.value() == indices.presentFamily.value()) {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = familyIndices;
    }

    VkResult res = vkCreateSwapchainKHR(m_vkDevice, &createInfo, nullptr, &m_vkSwapChain);
    if (res != VK_SUCCESS) {
        throw std::runtime_error(fmt::format("Failed to create vk swap chain ({})", string_VkResult(res)));
    }

    // Fetching swapChain images
    uint32_t imageCount = 0;
    res = vkGetSwapchainImagesKHR(m_vkDevice, m_vkSwapChain, &imageCount, nullptr);
    if (res != VK_SUCCESS) {
        throw std::runtime_error(fmt::format("Failed to count swap chain images ({})", string_VkResult(res)));
    }

    m_swapChainImages.resize(imageCount);

    res = vkGetSwapchainImagesKHR(m_vkDevice, m_vkSwapChain, &imageCount, m_swapChainImages.data());
    if (res != VK_SUCCESS) {
        throw std::runtime_error(fmt::format("Failed to get swap chain images ({})", string_VkResult(res)));
    }

    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainExtent = extent;
}

std::vector<VkImageView> VK::m_createImageViews() {
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

        VkResult res = vkCreateImageView(m_vkDevice, &createInfo, nullptr, &m_swapChainImageViews[i]);
        if (res != VK_SUCCESS) {
            throw std::runtime_error(fmt::format("Failed to create image view ({})", string_VkResult(res)));
        }
    }
}

void VK::m_createRenderPass() {
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

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    const VkResult res = vkCreateRenderPass(m_vkDevice, &renderPassInfo, nullptr, &m_renderPass);
    if (res != VK_SUCCESS) {
        throw std::runtime_error(fmt::format("Failed to create render pass! ({})", string_VkResult(res)));
    }
}

void VK::m_createGraphicsPipeline() {
    const Shader vertShader(m_vkDevice, "./shaders/vert.spv");
    const Shader fragShader(m_vkDevice, "./shaders/frag.spv");

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
        fragShaderStageInfo
    };

    VkPipelineVertexInputStateCreateInfo vtxInputInfo{};
    vtxInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    const std::vector dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = dynamicStates.size();
    dynamicStateInfo.pDynamicStates = dynamicStates.data();

    VkViewport viewport{};
    viewport.width = static_cast<float>(m_swapChainExtent.width);
    viewport.height = static_cast<float>(m_swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
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
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    VkResult res = vkCreatePipelineLayout(m_vkDevice, &pipelineLayoutInfo, nullptr, &m_vkPipelineLayout);
    if (res != VK_SUCCESS) {
        throw std::runtime_error(fmt::format("Failed to create pipeline layout! ({})", string_VkResult(res)));
    }

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
    pipelineInfo.pDynamicState = &dynamicStateInfo;
    pipelineInfo.layout = m_vkPipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;

    res = vkCreateGraphicsPipelines(m_vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline);
    if (res != VK_SUCCESS) {
        throw std::runtime_error(fmt::format("failed to create graphics pipeline! ({})", string_VkResult(res)));
    }
}

void VK::m_initVulkan(SDL_Window *window) {
    fmt::println("Initializing vk");

    m_createVKInstance(window);
    m_createSurface(window);
    m_pickPhysicalDevice();
    m_createLogicalDevice();
    m_createSwapChain(window);
    m_createRenderPass();
    m_createGraphicsPipeline();

    fmt::println("Good to go :)");
}

void VK::m_destroyVulkan() {
    vkDestroyPipeline(m_vkDevice, m_pipeline, nullptr);
    m_pipeline = VK_NULL_HANDLE;

    vkDestroyPipelineLayout(m_vkDevice, m_vkPipelineLayout, nullptr);
    m_vkPipelineLayout = VK_NULL_HANDLE;

    vkDestroyRenderPass(m_vkDevice, m_renderPass, nullptr);
    m_renderPass = VK_NULL_HANDLE;

    for (VkImageView imageView: m_swapChainImageViews) {
        vkDestroyImageView(m_vkDevice, imageView, nullptr);
    }
    m_swapChainImageViews.clear();

    vkDestroySwapchainKHR(m_vkDevice, m_vkSwapChain, nullptr);
    m_vkSwapChain = VK_NULL_HANDLE;

    vkDestroyDevice(m_vkDevice, nullptr);
    m_vkDevice = VK_NULL_HANDLE;

    vkDestroySurfaceKHR(m_vkInstance, m_vkSurface, nullptr);
    m_vkSurface = VK_NULL_HANDLE;

    vkDestroyInstance(m_vkInstance, nullptr);
    m_vkInstance = VK_NULL_HANDLE;

    fmt::println("Destroyed vk resources");
}
