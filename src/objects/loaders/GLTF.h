#pragma once

#include <string>
#include <variant>
#include <unordered_map>

namespace GLTF {
enum ComponentType {
    BYTE = 5120,
    UNSIGNED_BYTE = 5121,
    SHORT = 5122,
    UNSIGNED_SHORT = 5123,
    UNSIGNED_INT = 5125,
    FLOAT = 5126,
};

using DataVariant = std::variant<std::vector<int8_t>, std::vector<uint8_t>, std::vector<int16_t>, std::vector<uint16_t>,
                                 std::vector<uint32_t>, std::vector<float>>;

struct Primitive {
    DataVariant data;
    // DataType type;
    uint64_t count;
    uint64_t byteSize;
    uint8_t byteWidth;
};

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

struct DataType {
    enum Type { SCALAR, VEC2, VEC3, VEC4, MAT2, MAT3, MAT4 };

    Type type;
    uint8_t componentCount;
};

static const std::unordered_map<std::string, DataType> dataTypeMap = {
    { "SCALAR", { DataType::SCALAR, 1 } },
    { "VEC2", { DataType::VEC2, 2 } },
    { "VEC3", { DataType::VEC3, 3 } },
    { "VEC4", { DataType::VEC4, 4 } },
    { "MAT2", { DataType::MAT2, 4 } },
    { "MAT3", { DataType::MAT3, 9 } },
    { "MAT4", { DataType::MAT4, 16 } },
};
} // namespace GLTF
