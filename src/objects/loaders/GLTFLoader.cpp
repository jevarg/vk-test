#include "GLTFLoader.h"

#include <fmt/format.h>

#include <glm/gtc/type_ptr.hpp>
#include <sstream>

#include "GLTF.h"
#include "fmt/printf.h"
#include "objects/Material.h"
#include "objects/Node.h"

using json = nlohmann::json;

GLTFLoader::GLTFLoader(const char* filePath): m_filePath(filePath) {
    std::ifstream f(filePath);
    m_gltf = json::parse(f);
    f.close();

    const auto rootPath = std::filesystem::path(filePath).parent_path();
    _loadFiles(rootPath);

    uint64_t sceneId = m_gltf["scene"];
    for (uint64_t rootNodeId : m_gltf["scenes"][sceneId]["nodes"]) {
        auto rootNode = m_gltf["nodes"][rootNodeId];
        Node node;

        // Make node children and fill it

        uint64_t meshId = rootNode["mesh"];
        Mesh mesh = _buildMesh(meshId);
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

Mesh GLTFLoader::_buildMesh(uint64_t meshId) const {
    auto gltfMesh = m_gltf["meshes"][meshId];
    const std::string meshName = gltfMesh.value("name", "unnamed");
    std::vector<Primitive> primitives;

    for (const auto& primitive : gltfMesh["primitives"]) {
        const GLTF::Primitive positionsPrimitive = _getPrimitiveBuffer(primitive["attributes"], "POSITION");
        const GLTF::Primitive indicesPrimitive = _getPrimitiveBuffer(primitive, "indices");
        if (!positionsPrimitive.count || !indicesPrimitive.count) {
            fmt::printf("{}: Missing positions or indices for mesh {}!", m_filePath, meshId);
            continue;
        }

        const GLTF::Primitive normalsPrimitive = _getPrimitiveBuffer(primitive["attributes"], "NORMAL");
        const GLTF::Primitive texCoordsPrimitive = _getPrimitiveBuffer(primitive["attributes"], "TEXCOORD_0");

        std::vector<Vertex> vertices(positionsPrimitive.count);

        const auto& rawPositions = std::get<std::vector<float>>(positionsPrimitive.data);
        for (int i = 0; i < positionsPrimitive.count; ++i) {
            vertices[i].pos = glm::make_vec3(&rawPositions[i * 3]);
            vertices[i].color = { 1, 1, 1 }; // TODO: is this ok?
        }

        if (normalsPrimitive.count > 1) {
            const auto& rawNormals = std::get<std::vector<float>>(normalsPrimitive.data);
            for (int i = 0; i < positionsPrimitive.count; ++i) {
                vertices[i].normal = glm::make_vec3(&rawNormals[i * 3]);
            }
        }

        if (texCoordsPrimitive.count > 1) {
            const auto& rawTexCoords = std::get<std::vector<float>>(texCoordsPrimitive.data);
            for (int i = 0; i < positionsPrimitive.count; ++i) {
                vertices[i].texCoord = glm::make_vec2(&rawTexCoords[i * 2]);
            }
        }

        const auto rawIndices = std::get<std::vector<uint16_t>>(indicesPrimitive.data);
        std::vector<uint32_t> indices(indicesPrimitive.count);
        for (int i = 0; i < indicesPrimitive.count; ++i) {
            indices[i] = rawIndices[i];
        }

        // auto mesh = std::make_shared<Primitive>(meshName, vertices, indices);
        // meshes.emplace_back(mesh);

        Primitive newPrimitive(vertices, indices);
        if (primitive.contains("material")) {
            const GLTF::Material gltfMaterial = _getMaterial(primitive["material"]);
            newPrimitive.setMaterial(std::make_unique<Material>(gltfMaterial.name));
        }

        primitives.push_back(std::move(newPrimitive));
    }

    return Mesh(meshName, std::move(primitives));
}

GLTF::Primitive GLTFLoader::_getPrimitiveBuffer(const nlohmann::json& primitive, const char* key) const {
    if (!primitive.contains(key)) {
        return {};
    }

    const uint64_t accessorId = primitive[key];

    const json accessor = m_gltf["accessors"][accessorId];
    const uint64_t bufferViewId = accessor["bufferView"];
    const uint64_t count = accessor["count"];

    const json bufferView = m_gltf["bufferViews"][bufferViewId];
    const uint64_t bufferId = bufferView["buffer"];
    const uint64_t offset = bufferView.value("byteOffset", 0);
    const uint64_t byteSize = bufferView["byteLength"];

    const std::string type = accessor["type"];
    const GLTF::DataType dataType = GLTF::dataTypeMap.at(type);

    GLTF::Primitive p{};
    p.count = count;
    p.byteSize = byteSize;

    const uint8_t* buffer = m_files.buffers[bufferId].data();
    const GLTF::ComponentType componentType = accessor["componentType"];
    switch (componentType) {
        case GLTF::ComponentType::BYTE: {
            std::vector<int8_t> values(count * dataType.componentCount);
            std::memcpy(values.data(), &buffer[offset], byteSize);
            p.data = std::move(values);
            break;
        }
        case GLTF::ComponentType::UNSIGNED_BYTE: {
            std::vector<uint8_t> values(count * dataType.componentCount);
            std::memcpy(values.data(), &buffer[offset], byteSize);
            p.data = std::move(values);
            break;
        }
        case GLTF::ComponentType::SHORT: {
            std::vector<int16_t> values(count * dataType.componentCount);
            std::memcpy(values.data(), &buffer[offset], byteSize);
            p.data = std::move(values);
            break;
        }
        case GLTF::ComponentType::UNSIGNED_SHORT: {
            std::vector<uint16_t> values(count * dataType.componentCount);
            std::memcpy(values.data(), &buffer[offset], byteSize);
            p.data = std::move(values);
            break;
        }
        case GLTF::ComponentType::UNSIGNED_INT: {
            std::vector<uint32_t> values(count * dataType.componentCount);
            std::memcpy(values.data(), &buffer[offset], byteSize);
            p.data = std::move(values);
            break;
        }
        case GLTF::ComponentType::FLOAT: {
            std::vector<float> values(count * dataType.componentCount);
            std::memcpy(values.data(), &buffer[offset], byteSize);
            p.data = std::move(values);
            break;
        }

        default:
            throw std::runtime_error(
                fmt::format("GLTF: unsupported componentType: {}", static_cast<uint32_t>(componentType)));
    }

    return p;
}

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

