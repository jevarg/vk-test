#include "GLTFLoader.h"

#include <fmt/format.h>

#include <glm/gtc/type_ptr.hpp>
#include <sstream>

#include "GLTF.h"

using json = nlohmann::json;

GLTFLoader::GLTFLoader(const char* filePath) {
    std::ifstream f(filePath);
    m_gltf = json::parse(f);
    f.close();

    const auto rootPath = std::filesystem::path(filePath).parent_path();
    loadFiles(rootPath);

    uint64_t sceneId = m_gltf["scene"];
    for (uint64_t nodeId : m_gltf["scenes"][sceneId]["nodes"]) {
        auto node = m_gltf["nodes"][nodeId];

        uint64_t meshId = node["mesh"];
        auto gltfMesh = m_gltf["meshes"][meshId];
        const std::string meshName = gltfMesh.value("name", "unnamed");
        for (const auto& primitive : gltfMesh["primitives"]) {
            const GLTF::Primitive indicesPrimitive = getPrimitiveBuffer(primitive, "indices");
            const GLTF::Primitive positionsPrimitive = getPrimitiveBuffer(primitive["attributes"], "POSITION");
            const GLTF::Primitive normalsPrimitive = getPrimitiveBuffer(primitive["attributes"], "NORMAL");
            const GLTF::Primitive texCoordsPrimitive = getPrimitiveBuffer(primitive["attributes"], "TEXCOORD_0");

            const auto& rawPositions = std::get<std::vector<float>>(positionsPrimitive.data);
            const auto& rawNormals = std::get<std::vector<float>>(normalsPrimitive.data);
            const auto& rawTexCoords = std::get<std::vector<float>>(texCoordsPrimitive.data);

            std::vector<Vertex> vertices(positionsPrimitive.count);
            for (int i = 0; i < positionsPrimitive.count; ++i) {
                vertices[i].pos = glm::make_vec3(&rawPositions[i * 3]);
                vertices[i].normal = glm::make_vec3(&rawNormals[i * 3]);
                vertices[i].texCoord = glm::make_vec2(&rawTexCoords[i * 2]);
                vertices[i].color = { 1, 1, 1 }; // TODO: is this ok?
            }

            const auto& rawIndices = std::get<std::vector<uint16_t>>(indicesPrimitive.data);
            std::vector<uint32_t> indices(indicesPrimitive.count);
            for (int i = 0; i < indicesPrimitive.count; ++i) {
                indices[i] = rawIndices[i];
            }

            auto mesh = std::make_shared<Mesh>("", vertices, indices);
            meshes.emplace_back(mesh);

            return;  // TODO: more meshes
        }
    }
}

void GLTFLoader::loadFiles(const std::filesystem::path& rootPath) {
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

GLTF::Primitive GLTFLoader::getPrimitiveBuffer(const nlohmann::json& primitive, const char* key) {
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

    GLTF::Primitive p;
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