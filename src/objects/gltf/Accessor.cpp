#include "Accessor.h"

#include <fmt/format.h>

namespace GLTF {

uint32_t Accessor::getElementCount() const {
    switch (type) {
        case Type::Scalar:
            return 1;
        case Type::Vec2:
            return 2;
        case Type::Vec3:
            return 3;
        case Type::Vec4:
        case Type::Mat2:
            return 4;
        case Type::Mat3:
            return 9;
        case Type::Mat4:
            return 16;
    }

    return 0;
}

size_t Accessor::getComponentSize() const {
    switch (componentType) {
        case Primitive::Type::Byte:
            return sizeof(int8_t);
        case Primitive::Type::UnsignedByte:
            return sizeof(uint8_t);
        case Primitive::Type::Short:
            return sizeof(int16_t);
        case Primitive::Type::UnsignedShort:
            return sizeof(uint16_t);
        case Primitive::Type::UnsignedInt:
            return sizeof(uint32_t);
        case Primitive::Type::Float:
            return sizeof(float);

        default:
            throw std::runtime_error(fmt::format("Unknown component type {}", static_cast<int32_t>(componentType)));
    }
}
}  // namespace GLTF