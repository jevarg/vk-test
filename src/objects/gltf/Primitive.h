#pragma once

#include <variant>
#include <vector>

#include "json.hpp"

namespace GLTF {
struct Attribute;

using DataVariant = std::variant<std::vector<int8_t>, std::vector<uint8_t>, std::vector<int16_t>, std::vector<uint16_t>,
                                 std::vector<uint32_t>, std::vector<float>>;

struct Primitive {
    enum class Type {
        Invalid = -1,

        Byte = 5120,
        UnsignedByte = 5121,
        Short = 5122,
        UnsignedShort = 5123,
        UnsignedInt = 5125,
        Float = 5126,
    };

    // NLOHMANN_JSON_SERIALIZE_ENUM(Attribute, {
    //         {Attribute::Position, "POSITION"},
    //         {Attribute::Normal, "NORMAL"},
    //         {Attribute::Tangent, "TANGENT"},
    //         {Attribute::Texcoord, "TEXCOORD"},
    //         {Attribute::Color, "COLOR"},
    //         {Attribute::Joints, "JOINTS"},
    //         {Attribute::Weights, "WEIGHTS"}
    //     });

    // Type type;

    std::vector<Attribute> attributes = {};
    std::optional<uint64_t> indices;
    uint64_t material;
    uint64_t mode = 4;
};

void from_json(const nlohmann::json& json, Primitive& outPrimitive);
}  // namespace GLTF
