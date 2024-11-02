#include "Shader.h"

#include <fstream>
#include <fmt/format.h>

Shader::Shader(const VkDevice &device, const char *path): m_device(device) {
    fmt::println("Creating shader module from {}", path);
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error(fmt::format("Unable to open {}", path));
    }

    const std::streamsize fileSize = file.tellg();
    file.seekg(0);
    m_bytecode.resize(fileSize);
    file.read(m_bytecode.data(), fileSize);
    file.close();

    m_createModule();
}


Shader::~Shader() {
    vkDestroyShaderModule(m_device, m_module, nullptr);
}

const VkShaderModule& Shader::getModule() const {
    return m_module;
}

void Shader::m_createModule() {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = m_bytecode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(m_bytecode.data());

    if (vkCreateShaderModule(m_device, &createInfo, nullptr, &m_module) != VK_SUCCESS) {
        throw std::runtime_error("Unable to create shader module");
    }
}
