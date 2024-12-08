#pragma once
#include <vulkan/vulkan_core.h>

#include <vector>

#include "types/VulkanContext.h"

class Shader {
   public:
    explicit Shader(const VulkanContext &vkContext, const char *path);

    ~Shader();

    [[nodiscard]]
    const VkShaderModule &getModule() const;

   private:
    const VulkanContext &m_vkContext;

    std::vector<char> m_bytecode;
    VkShaderModule m_module = VK_NULL_HANDLE;

    void m_createModule();
};
