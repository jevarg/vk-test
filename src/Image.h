#pragma once

#include <vulkan/vulkan_core.h>

class Image {
   public:
    explicit Image(VkDevice device, VkPhysicalDevice physicalDevice, const VkExtent3D& extent, VkFormat format,
                   VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);

    void destroy() const;
    void transitionLayout(VkCommandPool commandPool, VkQueue queue, VkImageLayout newLayout);

    [[nodiscard]]
    const VkExtent3D& getExtent() const;

    [[nodiscard]]
    VkImage getImage() const;

    [[nodiscard]]
    VkImageLayout getLayout() const;

    [[nodiscard]]
    VkImageView getImageView() const;

   private:
    const VkDevice m_device;

    VkImage m_image = VK_NULL_HANDLE;
    VkDeviceMemory m_deviceMemory = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;

    VkExtent3D m_extent;
    VkImageLayout m_layout;
};
