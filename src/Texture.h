#pragma once

#include <vulkan/vulkan.h>

#include <memory>

#include "Image.h"

class Buffer;

class Texture {
   public:
    explicit Texture(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue,
                     const char* filename);

    void destroy() const;

    const Image& getImage() const;

   private:
    const VkDevice m_device;

    std::unique_ptr<Buffer> m_stagingBuffer;
    std::unique_ptr<Image> m_image;
};
