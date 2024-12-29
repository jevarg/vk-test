#pragma once

#include <vulkan/vulkan_core.h>

#include <shaderc/shaderc.hpp>
#include <vector>

#include "../types/VulkanContext.h"

class Shader {
   public:
    enum Type { Vertex = shaderc_vertex_shader, Fragment = shaderc_fragment_shader };

    explicit Shader(const char *path, Type shaderType);

    ~Shader();

    [[nodiscard]]
    const VkShaderModule &getModule() const;

   private:
    const char *m_filePath;
    const Type m_type;

    std::vector<uint32_t> m_bytecode;
    VkShaderModule m_module = VK_NULL_HANDLE;

    void m_compile(const std::string &glslString);
    void m_createModule();
};
