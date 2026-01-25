#pragma once

#include <string>
#include <unordered_map>

namespace GLTF {
struct Attribute {
    enum class Type {
        Position,
        Normal,
        Tangent,

        Texcoord,  //
        Color,
        Joints,
        Weights,
    };

    Type type;
    bool isUnique;
    uint32_t index;
    uint32_t accessorIndex;

    explicit Attribute(const std::string& rawType, uint32_t accessorIndex);
};

}  // namespace GLTF