#pragma once

#include <memory>
#include <span>
#include <string>
#include <initializer_list>

#include "Image.h"
#include "Buffer.h"

class Texture {
public:
    typedef size_t ID;

    explicit Texture(const std::string& filename, const VkDescriptorPool& descriptorPool,
                     const VkDescriptorSetLayout& descriptorSetLayout);
    explicit Texture(std::span<const std::string> filenames, const VkDescriptorPool& descriptorPool,
                     const VkDescriptorSetLayout& descriptorSetLayout);
    explicit Texture(std::initializer_list<std::string> filenames, const VkDescriptorPool& descriptorPool,
                     const VkDescriptorSetLayout& descriptorSetLayout);
    Texture(Texture&& other) noexcept = default;

    void destroy() const;

    // void bind(const VkCommandBuffer& commandBuffer, const VkPipelineLayout& pipelineLayout) const;

    [[nodiscard]]
    const Image& getImage() const;

    [[nodiscard]]
    ID getID() const;

    [[nodiscard]]
    const VkDescriptorSet& getDescriptorSet() const;

private:
    inline static ID lastID = 0;

    static ID nextID() {
        return lastID++;
    }

    const ID m_id = nextID();

    void m_createDescriptorSet(const VkDescriptorPool& descriptorPool,
                               const VkDescriptorSetLayout& descriptorSetLayout);
    void m_createSampler();

    std::unique_ptr<Buffer> m_stagingBuffer;
    std::unique_ptr<Image> m_image;

    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
    VkSampler m_sampler = VK_NULL_HANDLE;
};
