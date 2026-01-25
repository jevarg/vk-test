#include "Primitive.h"

#include <fmt/format.h>

#include "Attribute.h"

namespace GLTF {

// Primitive::Primitive(const Type type, const std::vector<uint8_t>& buffer) : type(type) {
//     setValues(buffer);
// }

// Primitive::Primitive(const Type type, std::vector<uint8_t> buffer) : type(type), rawValues(std::move(buffer)) {}
//
// void Primitive::setValues(const std::vector<uint8_t>& buffer) {
//     switch (type) {
//         case Type::Byte:
//             rawValues.emplace<std::vector<int8_t>>(buffer.begin(), buffer.end());
//             break;
//         case Type::UnsignedByte:
//             rawValues.emplace<std::vector<uint8_t>>(buffer.begin(), buffer.end());
//             break;
//         case Type::Short:
//             rawValues.emplace<std::vector<int16_t>>(buffer.begin(), buffer.end());
//             break;
//         case Type::UnsignedShort:
//             rawValues.emplace<std::vector<uint16_t>>(buffer.begin(), buffer.end());
//             break;
//         case Type::UnsignedInt:
//             rawValues.emplace<std::vector<uint32_t>>(buffer.begin(), buffer.end());
//             break;
//         case Type::Float:
//             rawValues.emplace<std::vector<float>>(buffer.begin(), buffer.end());
//             break;
//
//         case Type::Invalid:
//             throw std::runtime_error(fmt::format("GLTF: unsupported componentType: {}", static_cast<uint32_t>(type)));
//     }
// }


void from_json(const nlohmann::json& json, Primitive& outPrimitive) {
    const Primitive defaultPrimitive{};
    for (const auto& item : json["attributes"].items()) {
        outPrimitive.attributes.emplace_back(item.key(), item.value());
    }

    outPrimitive.mode = json.value("mode", defaultPrimitive.mode);
    outPrimitive.material = json.value("material", defaultPrimitive.material);
    outPrimitive.mode = json.value("mode", defaultPrimitive.mode);

    if (json.contains("indices")) {
        outPrimitive.indices = json["indices"];
    }
}

}  // namespace GLTF