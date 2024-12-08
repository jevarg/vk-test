#pragma once

#include <vulkan/vulkan_core.h>

#include <memory>

#include "Image.h"
#include "types/VulkanContext.h"

class Buffer;

class Texture {
   public:
    explicit Texture(const VulkanContext& vkContext, const char* filename);

    void destroy() const;

    [[nodiscard]]
    const Image& getImage() const;

   private:
    const VulkanContext& m_vkContext;

    std::unique_ptr<Buffer> m_stagingBuffer;
    std::unique_ptr<Image> m_image;
};
