#pragma once

#include <fmt/format.h>

#include <string>
#include <unordered_map>
#include <variant>

namespace GLTF {

struct MetallicRoughness {
    glm::vec4 baseColor{ 1, 1, 1, 1 };
    uint32_t baseColorTexture;
    float metallic = 1.0f;
    float roughness = 1.0f;
    uint32_t metallicRoughnessTexture;
};

struct Material {
    std::string name;
    MetallicRoughness metallicRoughness;
    uint32_t normalTexture;
    // uint32_t occlusionTexture;
    // uint32_t emissiveTexture;
};

// struct DataType {
//     enum class Type { SCALAR, VEC2, VEC3, VEC4, MAT2, MAT3, MAT4 };
//
//     Type type;
//     uint8_t componentCount;
// };
//
// static const std::unordered_map<std::string, DataType> dataTypeMap = {
//     { "SCALAR", { DataType::SCALAR, 1 } },
//     {   "VEC2",   { DataType::VEC2, 2 } },
//     {   "VEC3",   { DataType::VEC3, 3 } },
//     {   "VEC4",   { DataType::VEC4, 4 } },
//     {   "MAT2",   { DataType::MAT2, 4 } },
//     {   "MAT3",   { DataType::MAT3, 9 } },
//     {   "MAT4",  { DataType::MAT4, 16 } },
// };
}  // namespace GLTF
