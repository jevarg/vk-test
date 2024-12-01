#pragma once

#include <vulkan/vulkan.h>
#include <memory>

class Buffer;

class Texture {
public:
    explicit Texture(const VkDevice& device, const VkPhysicalDevice& physicalDevice, const VkCommandPool& commandPool, const VkQueue& queue, const char* filename);
    void destroy() const;

    [[nodiscard]]
    const VkExtent3D& getExtent() const;

    [[nodiscard]]
    const VkImage& getImage() const;

    [[nodiscard]]
    const VkImageLayout& getLayout() const;

    [[nodiscard]]
    const VkImageView& getImageView() const;

    void transitionLayout(const VkCommandPool& commandPool, const VkQueue& queue, const VkImageLayout newLayout);

private:
    const VkDevice& m_device;
    std::unique_ptr<Buffer> m_stagingBuffer;

    VkExtent3D m_extent = {};
    VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImage m_image = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
};
