#include "DepthImage.h"

DepthImage::DepthImage(const VulkanContext& vkContext, const VkExtent3D& extent, const VkFormat format)
    : Image(vkContext, extent, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT) {}