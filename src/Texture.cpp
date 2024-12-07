#define STB_IMAGE_IMPLEMENTATION

#include "Texture.h"

#include <fmt/format.h>
#include <stb_image.h>

#include <stdexcept>

#include "Buffer.h"

Texture::Texture(const VkDevice device, const VkPhysicalDevice physicalDevice, const VkCommandPool commandPool,
                 const VkQueue queue, const char *filename)
    : m_device(device) {
    int width = 0;
    int height = 0;
    int channels = 0;

    stbi_uc *pixels = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);
    if (pixels == nullptr) {
        throw std::runtime_error(fmt::format("failed to load texture {}", filename));
    }

    m_stagingBuffer = std::make_unique<Buffer>(
        m_device, physicalDevice, width * height * STBI_rgb_alpha, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    m_stagingBuffer->setMemory(pixels);
    stbi_image_free(pixels);

    VkExtent3D extent{};
    extent.width = width;
    extent.height = height;
    extent.depth = 1;

    m_image = std::make_unique<Image>(device, physicalDevice, extent, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                                      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

    m_image->transitionLayout(commandPool, queue, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    m_stagingBuffer->copyTo(*this, commandPool, queue);
    m_image->transitionLayout(commandPool, queue, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    m_stagingBuffer->destroy();

    fmt::println("Loaded texture: {}", filename);
}

void Texture::destroy() const {
    m_image->destroy();
}

const Image &Texture::getImage() const {
    return *m_image;
}
