#pragma once

#include "BufferView.h"
#include "Primitive.h"

namespace GLTF {
struct Accessor {
    template <typename T>
    using VisitorFunction = std::function<void(size_t, T)>;

    enum class Type { Scalar, Vec2, Vec3, Vec4, Mat2, Mat3, Mat4 };
    NLOHMANN_JSON_SERIALIZE_ENUM(Type, {
                                           { Type::Scalar, "SCALAR" },
                                           {   Type::Vec2,   "VEC2" },
                                           {   Type::Vec3,   "VEC3" },
                                           {   Type::Vec4,   "VEC4" },
                                           {   Type::Mat2,   "MAT2" },
                                           {   Type::Mat3,   "MAT3" },
                                           {   Type::Mat4,   "MAT4" }
    });

    BufferView bufferView;
    int64_t bufferOffset;
    Primitive::Type componentType;
    bool normalized;
    int64_t count;
    Type type;
    int64_t max;
    int64_t min;
    std::string name;
    size_t size;

    // DataVariant bytes;

    [[nodiscard]]
    uint32_t getElementCount() const;

    [[nodiscard]]
    size_t getComponentSize() const;

    template <typename T>
    std::span<const T> access(const std::vector<std::vector<uint8_t>>& buffers) const {
        auto ptr = reinterpret_cast<const T*>(&buffers[bufferView.buffer][bufferOffset]);
        return std::span<const T>(ptr, count);
    }

    template <typename T>
    void visit(const std::vector<std::vector<uint8_t>>& buffers, const VisitorFunction<T>& func) const {
        const std::span<const T>& buffer = access<const T>(buffers);
        for (int i = 0; i < buffer.size(); ++i) {
            func(i, buffer[i]);
        }
    }
};

inline void from_json(const nlohmann::json& json, Accessor& accessor) {
    const Accessor defaultObject{};

    accessor.bufferView = json.value("bufferView", defaultObject.bufferView);
    accessor.bufferOffset = json.value("bufferOffset", defaultObject.bufferOffset);
    accessor.componentType = json.value("componentType", defaultObject.componentType);
    accessor.normalized = json.value("normalized", defaultObject.normalized);
    accessor.count = json.value("count", defaultObject.count);
    accessor.type = json.value("type", defaultObject.type);
    accessor.max = json.value("max", defaultObject.max);
    accessor.min = json.value("min", defaultObject.min);
    accessor.name = json.value("name", defaultObject.name);

    accessor.size = accessor.count * accessor.getElementCount() * accessor.getComponentSize();
}
}  // namespace GLTF

