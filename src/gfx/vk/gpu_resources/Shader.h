#pragma once

#include <vulkan/vulkan_core.h>

#include <shaderc/shaderc.hpp>
#include <vector>

#include "../types/VulkanContext.h"

class Shader {
   public:
    enum Type { Vertex = shaderc_vertex_shader, Fragment = shaderc_fragment_shader };

    explicit Shader(const char *path, Type shaderType);

    void destroy() const;

    [[nodiscard]]
    const VkShaderModule &getModule() const;

    [[nodiscard]]
    const char* getEntryPoint() const;

   private:
    const char *m_filePath;
    const Type m_type;

    std::vector<uint32_t> m_bytecode;
    VkShaderModule m_module = VK_NULL_HANDLE;
    const char *m_entrypoint = "main";

    void m_compile(const std::string &glslString);
    void m_createModule();
};
