#include "Attribute.h"

#include <fmt/format.h>

#include <charconv>

namespace GLTF {

struct AttributeBase {
    Attribute::Type type;
    bool isUnique;
};

static std::unordered_map<std::string, AttributeBase> AttributesMap = {
    { "POSITION",  { Attribute::Type::Position, true } },
    {   "NORMAL",    { Attribute::Type::Normal, true } },
    {  "TANGENT",   { Attribute::Type::Tangent, true } },

    { "TEXCOORD", { Attribute::Type::Texcoord, false } },
    {    "COLOR",    { Attribute::Type::Color, false } },
    {   "JOINTS",   { Attribute::Type::Joints, false } },
    {  "WEIGHTS",  { Attribute::Type::Weights, false } },
};

Attribute::Attribute(const std::string& rawType, const uint32_t accessorIndex) : accessorIndex(accessorIndex) {
    const auto it =
        std::ranges::find_if(AttributesMap, [rawType](const auto& kv) { return rawType.starts_with(kv.first); });

    if (it == AttributesMap.end()) {
        throw std::runtime_error(fmt::format("Unknown attribute: {}", rawType));
    }

    type = it->second.type;
    isUnique = it->second.isUnique;

    if (!isUnique) {
        const std::string_view strIndex(rawType.data() + it->first.length() + 1);
        const auto result = std::from_chars(strIndex.begin(), strIndex.end(), index);
        if (result.ec != std::errc()) {
            throw std::runtime_error(fmt::format("Invalid attribute: {}", rawType));
        }
    }
}
}  // namespace GLTF
