#pragma once

#include <SDL_events.h>
#include <SDL_video.h>
#include <vulkan/vulkan_core.h>

namespace Debug::View {

void init(SDL_Window* window, VkInstance instance, VkRenderPass renderPass, uint32_t imageCount);
void destroy();

void beginFrame();
void endFrame(VkCommandBuffer commandBuffer);

[[nodiscard]]
bool wantsKeyboard();

[[nodiscard]]
bool isVisible();
bool toggle();
void update(const SDL_Event* event);

}  // namespace Debug::View
