#pragma once

#include <json.hpp>
#include <string>

namespace GLTF {
struct BufferView {
    uint64_t buffer;
    uint64_t byteOffset;
    uint64_t byteLength;
    uint64_t byteStride;

    std::string name;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(BufferView, buffer, byteOffset, byteLength, byteStride, name);
};
}  // namespace GLTF