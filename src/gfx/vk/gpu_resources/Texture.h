#pragma once

#include <memory>
#include <span>
#include <string>
#include <initializer_list>

#include "Image.h"
#include "Buffer.h"

class Texture {
public:
    explicit Texture(const std::string& filename, const VkDescriptorPool& descriptorPool,
                     const VkDescriptorSetLayout& descriptorSetLayout);
    explicit Texture(std::span<const std::string> filenames, const VkDescriptorPool& descriptorPool,
                     const VkDescriptorSetLayout& descriptorSetLayout);
    explicit Texture(std::initializer_list<std::string> filenames, const VkDescriptorPool& descriptorPool,
                     const VkDescriptorSetLayout& descriptorSetLayout);
    Texture(Texture&& other) noexcept = default;

    void destroy() const;

    [[nodiscard]]
    const Image& getImage() const;

    [[nodiscard]]
    const VkDescriptorSet& getDescriptorSet() const;

private:
    void m_createDescriptorSet(const VkDescriptorPool& descriptorPool,
                               const VkDescriptorSetLayout& descriptorSetLayout);
    void m_createSampler();

    std::unique_ptr<Buffer> m_stagingBuffer;
    std::unique_ptr<Image> m_image;

    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
    VkSampler m_sampler = VK_NULL_HANDLE;
};
