#define STB_IMAGE_IMPLEMENTATION

#include "Texture.h"

#include <fmt/format.h>
#include <stb_image.h>

#include <stdexcept>

#include "Buffer.h"
#include "gfx/vk/vkutil.h"

Texture::Texture(const std::vector<const char *> &filenames, const VkDescriptorPool &descriptorPool,
                 const VkDescriptorSetLayout &descriptorSetLayout) {
    int width = 0;
    int height = 0;
    int channels = 0;

    uint32_t layersCount = filenames.size();
    uint8_t *mappedBuffer = nullptr;

    for (int i = 0; i < layersCount; ++i) {
        stbi_uc *pixels = stbi_load(filenames[i], &width, &height, &channels, STBI_rgb_alpha);
        if (pixels == nullptr) {
            throw std::runtime_error(fmt::format("failed to load texture {}", filenames[i]));
        }

        const uint32_t layerSize = width * height * STBI_rgb_alpha;

        if (i == 0) {
            // NOTE: Hoping that all layers are the same size otherwise we're f****d!
            const uint32_t bufferSize = layerSize * layersCount;
            m_stagingBuffer =
                std::make_unique<Buffer>(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            mappedBuffer = static_cast<uint8_t *>(m_stagingBuffer->map());
        }

        memcpy(mappedBuffer, pixels, layerSize);
        stbi_image_free(pixels);

        mappedBuffer += layerSize;
    }

    m_stagingBuffer->unmap();

    VkExtent3D extent{};
    extent.width = width;
    extent.height = height;
    extent.depth = 1;

    // TODO: Is this ok?
    VkImageViewType imageViewType = layersCount == 6 ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
    m_image = std::make_unique<Image>(extent, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                                      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT,
                                      imageViewType, 1, layersCount);

    m_image->transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    m_stagingBuffer->copyTo(*this, layersCount);
    m_image->transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    m_stagingBuffer->destroy();

    m_createSampler();
    m_createDescriptorSet(descriptorPool, descriptorSetLayout);
}

void Texture::m_createDescriptorSet(const VkDescriptorPool &descriptorPool,
                                    const VkDescriptorSetLayout &descriptorSetLayout) {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    VK_CHECK("Cannot create descriptor set",
             vkAllocateDescriptorSets(VulkanContext::get().getDevice(), &allocInfo, &m_descriptorSet));

    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = m_sampler;
    imageInfo.imageLayout = m_image->getLayout();
    imageInfo.imageView = m_image->getImageView();

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(VulkanContext::get().getDevice(), 1, &descriptorWrite, 0, nullptr);
}

void Texture::m_createSampler() {
    const VulkanContext &vkContext = VulkanContext::get();
    const VkPhysicalDeviceProperties &properties = vkContext.getPhysicalDevice().getProperties();

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VK_CHECK("failed to create sampler", vkCreateSampler(vkContext.getDevice(), &samplerInfo, nullptr, &m_sampler));
}

void Texture::destroy() const {
    m_image->destroy();
    vkDestroySampler(VulkanContext::get().getDevice(), m_sampler, nullptr);
}

// void Texture::bind(const VkCommandBuffer &commandBuffer, const VkPipelineLayout &pipelineLayout) const {
//     vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
//                             &m_descriptorSet, 0, nullptr);
// }

const Image &Texture::getImage() const {
    return *m_image;
}

size_t Texture::getID() const {
    return m_id;
}

const VkDescriptorSet &Texture::getDescriptorSet() const {
    return m_descriptorSet;
}
