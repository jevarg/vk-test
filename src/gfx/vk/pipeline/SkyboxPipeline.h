#pragma once

#include "Pipeline.h"

class SkyboxPipeline : public Pipeline {
public:
    explicit SkyboxPipeline(const VkExtent2D& extent, const std::span<VkDescriptorSetLayout>& setLayouts,
                            VkRenderPass renderPass);
};

