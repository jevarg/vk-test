#pragma once

#include <memory>

#include "../types/VulkanContext.h"
#include "Image.h"

class Buffer;

class Texture {
   public:
    Texture(const VulkanContext& vkContext, const char* filename);
    Texture(const Texture& texture) = default;

    void destroy() const;

    [[nodiscard]]
    const Image& getImage() const;

   private:
    const VulkanContext& m_vkContext;

    std::unique_ptr<Buffer> m_stagingBuffer;
    std::unique_ptr<Image> m_image;
};
