#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>

class Shader {
public:
    explicit Shader(const VkDevice &device, const char *path);

    ~Shader();

    const VkShaderModule &getModule() const;



private:
    void m_createModule();

    std::vector<char> m_bytecode;
    const VkDevice &m_device;
    VkShaderModule m_module = VK_NULL_HANDLE;
};
