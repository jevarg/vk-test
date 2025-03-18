#pragma once

#include <optional>
#include <string>

#include "gfx/vk/gpu_resources/Texture.h"

class Material {
public:
    Material(const std::string& name);

private:
    std::string m_name;

    std::optional<Texture> baseColorTexture;
    std::optional<Texture> metallicRoughnessTexture;
};
