#define STB_IMAGE_IMPLEMENTATION

#include "Texture.h"

#include <fmt/format.h>
#include <stb_image.h>

#include <stdexcept>

#include "Buffer.h"

Texture::Texture(const char *filename) {
    int width = 0;
    int height = 0;
    int channels = 0;

    stbi_uc *pixels = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);
    if (pixels == nullptr) {
        throw std::runtime_error(fmt::format("failed to load texture {}", filename));
    }

    m_stagingBuffer =
        std::make_unique<Buffer>(width * height * STBI_rgb_alpha, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    m_stagingBuffer->setMemory(pixels);
    stbi_image_free(pixels);

    VkExtent3D extent{};
    extent.width = width;
    extent.height = height;
    extent.depth = 1;

    m_image = std::make_unique<Image>(extent, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                                      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

    m_image->transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    m_stagingBuffer->copyTo(*this);
    m_image->transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    m_stagingBuffer->destroy();

    fmt::println("Loaded texture: {}", filename);
}

// void Texture::createDescriptorSet(VkSampler sampler) {
//     VkDescriptorImageInfo imageInfo{};
//     imageInfo.sampler = sampler;
//     imageInfo.imageLayout = m_image->getLayout();
//     imageInfo.imageView = m_image->getImageView();
//
//     descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//     descriptorWrites[1].dstSet = m_descriptorSet;
//     descriptorWrites[1].dstBinding = 1;
//     descriptorWrites[1].dstArrayElement = 0;
//     descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//     descriptorWrites[1].descriptorCount = 1;
//     descriptorWrites[1].pImageInfo = &imageInfo;
// }

void Texture::destroy() const {
    m_image->destroy();
}

const Image &Texture::getImage() const {
    return *m_image;
}

size_t Texture::getID() const {
    return m_id;
}

// VkDescriptorSet Texture::getDescriptorSet() const {
//     return m_descriptorSet;
// }
