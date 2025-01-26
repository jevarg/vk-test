#include "Shader.h"

#include <fmt/format.h>
#include <fstream>

Shader::Shader(const char *path, const Type shaderType) : m_filePath(path), m_type(shaderType) {
    std::ifstream file(m_filePath, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error(fmt::format("Unable to open {}", m_filePath));
    }

    const std::streamsize fileSize = file.tellg();
    file.seekg(0);

    std::string glslString;
    glslString.reserve(fileSize);
    glslString.assign((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
    file.close();

    m_compile(glslString);
    m_createModule();
}

void Shader::destroy() const {
    vkDestroyShaderModule(VulkanContext::get().getDevice(), m_module, nullptr);
}

const VkShaderModule &Shader::getModule() const {
    return m_module;
}

const char* Shader::getEntryPoint() const {
    return m_entrypoint;
}

void Shader::m_compile(const std::string &glslString) {
    const shaderc::Compiler compiler;
    const shaderc::SpvCompilationResult res =
            compiler.CompileGlslToSpv(glslString, static_cast<shaderc_shader_kind>(m_type), m_filePath);

    if (res.GetCompilationStatus() != shaderc_compilation_status_success) {
        throw std::runtime_error(fmt::format("Could not compile {}: {}", m_filePath, res.GetErrorMessage()));
    }

    fmt::println("Compiled {} with {} warning(s)", m_filePath, res.GetNumWarnings());
    m_bytecode.assign(res.begin(), res.end());
}

void Shader::m_createModule() {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = sizeof(m_bytecode[0]) * m_bytecode.size();
    createInfo.pCode = m_bytecode.data();

    if (vkCreateShaderModule(VulkanContext::get().getDevice(), &createInfo, nullptr, &m_module) != VK_SUCCESS) {
        throw std::runtime_error("Unable to create shader module");
    }
}
