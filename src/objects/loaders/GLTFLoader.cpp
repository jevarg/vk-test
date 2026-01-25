#include "GLTFLoader.h"

#include <fmt/format.h>

#include <glm/gtc/type_ptr.hpp>
#include <sstream>

#include "GLTF.h"
#include "fmt/printf.h"
#include "objects/Material.h"
#include "objects/Model.h"
#include "objects/Node.h"
#include "objects/gltf/Accessor.h"
#include "objects/gltf/Attribute.h"
#include "objects/gltf/Primitive.h"

using json = nlohmann::json;

GLTFLoader::GLTFLoader(const char* filePath) : m_filePath(filePath) {
    std::ifstream f(filePath);
    if (!f.is_open()) {
        throw std::runtime_error(fmt::format("GLTF ERROR: cannot open {}", filePath));
    }

    m_gltf = json::parse(f);
    f.close();

    const auto rootPath = std::filesystem::path(filePath).parent_path();
    _loadFiles(rootPath);

    m_accessors = m_gltf["accessors"].get<std::vector<GLTF::Accessor>>();

    uint64_t sceneId = m_gltf["scene"];
    for (uint64_t rootNodeId : m_gltf["scenes"][sceneId]["nodes"]) {
        auto rootNode = m_gltf["nodes"][rootNodeId];

        // Make node children and fill it

        uint64_t meshId = rootNode["mesh"];
        std::unique_ptr<Node> node = std::make_unique<Node>(_buildMesh(meshId));
        m_model = std::make_unique<Model>(std::move(node), 0);
        // auto gltfMesh = m_gltf["meshes"][meshId];
        // const std::string meshName = gltfMesh.value("name", "unnamed");
        // for (const auto& primitive : gltfMesh["primitives"]) {
        //     const GLTF::Primitive positionsPrimitive = _getPrimitiveBuffer(primitive["attributes"], "POSITION");
        //     const GLTF::Primitive indicesPrimitive = _getPrimitiveBuffer(primitive, "indices");
        //     if (!positionsPrimitive.count || !indicesPrimitive.count) {
        //         fmt::printf("{}: Missing positions or indices for mesh {}!", filePath, meshId);
        //         continue;
        //     }
        //
        //     const GLTF::Primitive normalsPrimitive = _getPrimitiveBuffer(primitive["attributes"], "NORMAL");
        //     const GLTF::Primitive texCoordsPrimitive = _getPrimitiveBuffer(primitive["attributes"], "TEXCOORD_0");
        //
        //     std::vector<Vertex> vertices(positionsPrimitive.count);
        //
        //     const auto& rawPositions = std::get<std::vector<float>>(positionsPrimitive.data);
        //     for (int i = 0; i < positionsPrimitive.count; ++i) {
        //         vertices[i].pos = glm::make_vec3(&rawPositions[i * 3]);
        //         vertices[i].color = { 1, 1, 1 }; // TODO: is this ok?
        //     }
        //
        //     if (normalsPrimitive.count > 1) {
        //         const auto& rawNormals = std::get<std::vector<float>>(normalsPrimitive.data);
        //         for (int i = 0; i < positionsPrimitive.count; ++i) {
        //             vertices[i].normal = glm::make_vec3(&rawNormals[i * 3]);
        //         }
        //     }
        //
        //     if (texCoordsPrimitive.count > 1) {
        //         const auto& rawTexCoords = std::get<std::vector<float>>(texCoordsPrimitive.data);
        //         for (int i = 0; i < positionsPrimitive.count; ++i) {
        //             vertices[i].texCoord = glm::make_vec2(&rawTexCoords[i * 2]);
        //         }
        //     }
        //
        //     const auto rawIndices = std::get<std::vector<uint16_t>>(indicesPrimitive.data);
        //     std::vector<uint32_t> indices(indicesPrimitive.count);
        //     for (int i = 0; i < indicesPrimitive.count; ++i) {
        //         indices[i] = rawIndices[i];
        //     }
        //
        //     auto mesh = std::make_shared<Mesh>(meshName, vertices, indices);
        //     // meshes.emplace_back(mesh);
        //
        //     if (primitive.contains("material")) {
        //         const GLTF::Material gltfMaterial = _getMaterial(primitive["material"]);
        //         Material material(gltfMaterial.name);
        //     }
        //
        //     // return; // TODO: more meshes
        // }
    }
}

std::unique_ptr<Model> GLTFLoader::acquire() {
    return std::move(m_model);
}

void GLTFLoader::_loadFiles(const std::filesystem::path& rootPath) {
    for (const auto& buffer : m_gltf["buffers"]) {
        std::ifstream bufferFile(rootPath / buffer["uri"], std::ios_base::binary);
        std::vector<uint8_t> data(std::istreambuf_iterator{ bufferFile }, {});
        m_files.buffers.push_back(data);

        bufferFile.close();
    }

    for (const auto& image : m_gltf["images"]) {
        std::ifstream bufferFile(rootPath / image["uri"], std::ios_base::binary);
        std::vector<uint8_t> data(std::istreambuf_iterator{ bufferFile }, {});
        m_files.images.push_back(data);

        bufferFile.close();
    }
}

std::unique_ptr<Mesh> GLTFLoader::_buildMesh(uint64_t meshId) const {
    auto gltfMesh = m_gltf["meshes"][meshId];
    const std::string meshName = gltfMesh.value("name", "unnamed");

    for (const GLTF::Primitive& primitive : gltfMesh["primitives"].get<std::vector<GLTF::Primitive>>()) {
        if (!primitive.indices.has_value()) {
            throw std::runtime_error("No indices, panicking!");
        }

        const GLTF::Accessor& indicesAccessor = m_accessors[primitive.indices.value()];
        std::vector<Vertex> vertices;

        const std::span rawIndices = indicesAccessor.access<uint16_t>(m_files.buffers);
        std::vector<uint32_t> indices(rawIndices.data(), rawIndices.data() + rawIndices.size_bytes());

        for (const GLTF::Attribute& attribute : primitive.attributes) {
            const GLTF::Accessor& accessor = m_accessors[attribute.accessorIndex];

            switch (attribute.type) {
                case (GLTF::Attribute::Type::Position):
                    accessor.visit<glm::vec3>(m_files.buffers, [&vertices](const size_t i, const glm::vec3 v) {
                        if (vertices.size() <= i) {
                            vertices.emplace_back();
                        }

                        vertices[i].pos = v;
                    });
                    break;

                case (GLTF::Attribute::Type::Normal):
                    accessor.visit<glm::vec3>(m_files.buffers, [&vertices](const size_t i, const glm::vec3 v) {
                        if (vertices.size() <= i) {
                            vertices.emplace_back();
                        }

                        vertices[i].normal = v;
                    });
                    break;

                case (GLTF::Attribute::Type::Texcoord):
                    accessor.visit<glm::vec3>(m_files.buffers, [&vertices](const size_t i, const glm::vec3 v) {
                        if (vertices.size() <= i) {
                            vertices.emplace_back();
                        }

                        vertices[i].texCoord = v;
                    });
                    break;
                default:
                    fmt::println("Ignored attribute type={}, isUnique={}, index={}, accessorIndex={}",
                                 static_cast<uint32_t>(attribute.type), attribute.isUnique, attribute.index,
                                 attribute.accessorIndex);
            }
        }
        // if (!positionsPrimitive.count || !indicesPrimitive.count) {
        //     fmt::printf("{}: Missing positions or indices for mesh {}!", m_filePath, meshId);
        //     continue;
        // }
        //
        // const GLTF::Primitive normalsPrimitive = _getAccessors(primitive["attributes"], "NORMAL");
        // const GLTF::Primitive texCoordsPrimitive = _getAccessors(primitive["attributes"], "TEXCOORD_0");
        //
        // std::vector<Vertex> vertices(positionsPrimitive.count);
        //
        // const auto& rawPositions = std::get<std::vector<float>>(positionsPrimitive.data);
        // // positionsPrimitive.buffer.;
        // for (int i = 0; i < positionsPrimitive.count; ++i) {
        //     vertices[i].pos = glm::make_vec3(&rawPositions[i * 3]);
        //     vertices[i].color = { 1, 1, 1 };  // TODO: is this ok?
        // }
        //
        // if (normalsPrimitive.count > 1) {
        //     const auto& rawNormals = std::get<std::vector<float>>(normalsPrimitive.data);
        //     for (int i = 0; i < positionsPrimitive.count; ++i) {
        //         vertices[i].normal = glm::make_vec3(&rawNormals[i * 3]);
        //     }
        // }
        //
        // if (texCoordsPrimitive.count > 1) {
        //     const auto& rawTexCoords = std::get<std::vector<float>>(texCoordsPrimitive.data);
        //     for (int i = 0; i < positionsPrimitive.count; ++i) {
        //         vertices[i].texCoord = glm::make_vec2(&rawTexCoords[i * 2]);
        //     }
        // }
        //
        // const auto rawIndices = std::get<std::vector<uint16_t>>(indicesPrimitive.data);
        // std::vector<uint32_t> indices(indicesPrimitive.count);
        // for (int i = 0; i < indicesPrimitive.count; ++i) {
        //     indices[i] = rawIndices[i];
        // }
        //
        // Primitive newPrimitive(std::move(vertices), std::move(indices));
        // if (primitive.contains("material")) {
        //     const GLTF::Material gltfMaterial = _getMaterial(primitive["material"]);
        //     newPrimitive.setMaterial(std::make_unique<Material>(gltfMaterial.name));
        // }
        //
        // primitives.push_back(std::move(newPrimitive));
    }

    std::vector<Primitive> primitives;
    return std::make_unique<Mesh>(meshName, std::move(primitives));
}

// GLTF::Accessor GLTFLoader::_getAccessors(const nlohmann::json& primitive) const {
//     for (auto accessor : primitive.get<std::vector<GLTF::Accessor>>()) {}
//     if (!primitive.contains(key)) {
//         return {};
//     }
//
//     const uint64_t accessorId = primitive[key];
//
//     GLTF::Accessor accessor = m_gltf["accessors"][accessorId].get<GLTF::Accessor>();
    // const uint64_t bufferViewId = accessor["bufferView"];
    // const uint64_t count = accessor["count"];

    // const json bufferView = m_gltf["bufferViews"][accessor.bufferView];
    // const uint64_t bufferId = bufferView["buffer"];
    // const uint64_t offset = bufferView.value("byteOffset", 0);
    // const uint64_t byteSize = bufferView["byteLength"];

    // const std::string type = accessor["type"];
    // const GLTF::DataType dataType = GLTF::dataTypeMap.at(accessor.type);

    // const GLTF::Primitive::Type componentType = accessor.componentType;
    // const uint8_t* firstElement = &m_files.buffers[bufferId][offset];
    // const uint8_t* lastElement = firstElement + accessor.size;

    // std::vector values(firstElement, lastElement);
    // GLTF::Primitive p(accessor.componentType, std::move(values));
    // TODO Primitive should not be used. it only contains a buffer? check this

    // std::vector<std::any> values(firstElement, lastElement);
    // GLTF::DataVariant values(firstElement, lastElement);
    // p.data = std::move(values);
    // template<typename T>
    // std::vector<T> values;
    // switch (componentType) {
    //     case GLTF::ComponentType::BYTE: {
    //         std::vector<int8_t> values(count * dataType.componentCount);
    //         std::memcpy(values.data(), firstElement, byteSize);
    //         p.data = std::move(values);
    //         break;
    //     }
    //     case GLTF::ComponentType::UNSIGNED_BYTE: {
    //         std::vector<uint8_t> values(count * dataType.componentCount);
    //         std::memcpy(values.data(), firstElement, byteSize);
    //         p.data = std::move(values);
    //         break;
    //     }
    //     case GLTF::ComponentType::SHORT: {
    //         std::vector<int16_t> values(count * dataType.componentCount);
    //         std::memcpy(values.data(), firstElement, byteSize);
    //         p.data = std::move(values);
    //         break;
    //     }
    //     case GLTF::ComponentType::UNSIGNED_SHORT: {
    //         std::vector<uint16_t> values(count * dataType.componentCount);
    //         std::memcpy(values.data(), firstElement, byteSize);
    //         p.data = std::move(values);
    //         break;
    //     }
    //     case GLTF::ComponentType::UNSIGNED_INT: {
    //         std::vector<uint32_t> values(count * dataType.componentCount);
    //         std::memcpy(values.data(), firstElement, byteSize);
    //         p.data = std::move(values);
    //         break;
    //     }
    //     case GLTF::ComponentType::FLOAT: {
    //         std::vector<float> values(firstElement, firstElement + count * dataType.componentCount);
    //         // for (int i = 0; i < MAX; ++i) {
    //         //
    //         // }
    //         std::memcpy(values.data(), firstElement, byteSize);
    //         p.data = std::move(values);
    //         break;
    //     }
    //
    //     default:
    //         throw std::runtime_error(
    //             fmt::format("GLTF: unsupported componentType: {}", static_cast<uint32_t>(componentType)));
    // }
//
//     return p;
// }

GLTF::Material GLTFLoader::_getMaterial(uint64_t materialId) const {
    json rawMaterial = m_gltf["materials"][materialId];
    GLTF::Material material{ rawMaterial.value("name", "Unnamed Material") };

    json pbrMaterial = rawMaterial["pbrMetallicRoughness"];
    if (pbrMaterial != nullptr) {
        json j = pbrMaterial["baseColorFactor"];
        if (j != nullptr) {
            const std::vector<float> baseColor = j.get<std::vector<float>>();
            material.metallicRoughness.baseColor = glm::make_vec4(baseColor.data());
        }

        j = pbrMaterial["baseColorTexture"];
        if (j != nullptr) {
            material.metallicRoughness.baseColorTexture = j["index"];
        } else {
            fmt::println("warning: baseColorTexture was not defined for material {}", materialId);
        }

        j = pbrMaterial["metallicRoughnessTexture"];
        if (j != nullptr) {
            material.metallicRoughness.metallicRoughnessTexture = j["index"];
        } else {
            fmt::println("warning: metallicRoughnessTexture was not defined for material {}", materialId);
        }

        material.metallicRoughness.metallic = pbrMaterial.value("metallicFactor", 1.0f);
        material.metallicRoughness.roughness = pbrMaterial.value("roughnessFactor", 1.0f);
    }

    json normal = rawMaterial["normalTexture"];
    if (normal != nullptr) {
        material.normalTexture = normal["index"];
    }

    return material;
}

