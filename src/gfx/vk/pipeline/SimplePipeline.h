#pragma once

#include "Pipeline.h"

class SimplePipeline : public Pipeline {
   public:
    explicit SimplePipeline(const VkExtent2D& extent, const std::span<VkDescriptorSetLayout>& setLayouts,
                            VkRenderPass renderPass);
};

