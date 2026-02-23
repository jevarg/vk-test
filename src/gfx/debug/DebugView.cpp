#include "DebugView.h"

#include <fmt/base.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>

#include "gfx/vk/types/VulkanContext.h"
#include "gfx/vk/vkutil.h"

namespace Debug::View {

namespace {
bool m_isVisible;
}

void init(SDL_Window* window, VkInstance instance, VkRenderPass renderPass, const uint32_t imageCount) {
    fmt::println("Initializing ImGui");
    const auto& vkContext = VulkanContext::get();
    const auto vkPhysicalDevice = vkContext.getPhysicalDevice();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplSDL2_InitForVulkan(window);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.ApiVersion = VK_API_VERSION_1_0;
    init_info.Instance = instance;
    init_info.PhysicalDevice = vkPhysicalDevice.getUnderlying();
    init_info.Device = vkContext.getDevice();
    init_info.QueueFamily = vkPhysicalDevice.getQueueFamilyIndices().graphicsFamily.value();
    init_info.Queue = vkContext.getGraphicsQueue();
    init_info.DescriptorPoolSize = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE;
    init_info.MinImageCount = imageCount;
    init_info.ImageCount = imageCount;
    init_info.PipelineInfoMain.RenderPass = renderPass;
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.CheckVkResultFn = [](const VkResult vk_result) { VK_CHECK("Imgui error", vk_result); };

    ImGui_ImplVulkan_Init(&init_info);
}

void destroy() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void beginFrame() {
    if (!m_isVisible) {
        return;
    }

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();
}

void endFrame(VkCommandBuffer commandBuffer) {
    if (!m_isVisible) {
        return;
    }

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

bool isVisible() {
    return m_isVisible;
}

bool toggle() {
    m_isVisible = !m_isVisible;
    return m_isVisible;
}

void update(const SDL_Event* event) {
    ImGui_ImplSDL2_ProcessEvent(event);
}

bool wantsKeyboard() {
    return ImGui::GetIO().WantCaptureKeyboard;
}

}  // namespace Debug::View