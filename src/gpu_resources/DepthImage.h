#pragma once

#include <vulkan/vulkan_core.h>

#include "Image.h"
#include "types/VulkanContext.h"

class DepthImage : public Image {
   public:
    explicit DepthImage(const VulkanContext& vkContext, const VkExtent3D& extent,
                        VkFormat format = VK_FORMAT_D32_SFLOAT /* TODO: findDepthFormat() */);
};
