#include "Model.h"

#include <fmt/base.h>

#include <fstream>
#include <iostream>
#include <json.hpp>
#include <sstream>

#include "gfx/vk/types/ModelConstants.h"

using json = nlohmann::json;

// void getBufferViews(const json::basic_json& gltf, const uint64_t accessorId) {
//     const auto accessor = gltf["accessors"][accessorId];
//
// }

// Model::Model(const char *filePath): m_textureID(0) {
//     std::ifstream f(filePath);
//     json gltf = json::parse(f);
//     f.close();
//
//     const auto fileDir = std::filesystem::path(filePath).parent_path();
//     std::vector<std::ifstream> gltfBuffers;
//     for (const auto& buffer : gltf["buffers"]) {
//         std::ifstream bufferFile(fileDir / buffer["uri"]);
//         gltfBuffers.push_back(std::move(bufferFile));
//     }
//
//     std::vector<std::ifstream> gltfImages;
//     for (const auto& image : gltf["images"]) {
//         std::ifstream imageFile(fileDir / image["uri"]);
//         gltfImages.push_back(std::move(imageFile));
//     }
//
//
//     uint64_t sceneId = gltf["scene"];
//     for (uint64_t nodeId : gltf["scenes"][sceneId]["nodes"]) {
//         auto node = gltf["nodes"][nodeId];
//
//         uint64_t meshId = node["mesh"];
//         auto gltfMesh = gltf["meshes"][meshId];
//         // const std::string meshName = gltfMesh["name"];
//         for (const auto& primitives : gltfMesh["primitives"]) {
//             // TODO: Make the following generic
//             // uint64_t texCoordsAccessorId = primitives["attributes"]["TEXCOORD_0"]; // TODO: handle TEXCOORD_n
//             uint64_t verticesAccessorId = primitives["attributes"]["POSITION"];
//             uint64_t normalsAccessorId = primitives["attributes"]["NORMAL"];
//             uint64_t indicesAccessorId = primitives["indices"];
//
//             // auto texCoordsAccessor = gltf["accessors"][texCoordsAccessorId];
//             auto verticesAccessor = gltf["accessors"][verticesAccessorId];
//             auto indicesAccessor = gltf["accessors"][indicesAccessorId];
//             auto normalsAccessor = gltf["accessors"][normalsAccessorId];
//
//             // TODO: check "componentType" and "type"
//             // uint64_t texCoordsBufferViewId = texCoordsAccessor["bufferView"];
//             uint64_t verticesBufferViewId = verticesAccessor["bufferView"];
//             uint64_t indicesBufferViewId = indicesAccessor["bufferView"];
//             uint64_t normalsBufferViewId = normalsAccessor["bufferView"];
//
//             // uint64_t texCoordsCount = texCoordsAccessor["count"];
//             uint64_t verticesCount = verticesAccessor["count"];
//             uint64_t indicesCount = indicesAccessor["count"];
//             uint64_t normalsCount = normalsAccessor["count"];
//
//             // auto texCoordsBufferView = gltf["bufferViews"][texCoordsBufferViewId];
//             auto verticesBufferView = gltf["bufferViews"][verticesBufferViewId];
//             auto indicesBufferView = gltf["bufferViews"][indicesBufferViewId];
//             auto normalsBufferView = gltf["bufferViews"][normalsBufferViewId];
//
//             std::ifstream& file = gltfBuffers[verticesBufferView["buffer"]];
//             std::stringstream strStream;
//
//             strStream << file.rdbuf();
//             auto fileContent = strStream.str().c_str();
//
//             // int64_t offset = verticesBufferView["byteOffset"];
//             // file.seekg(offset);
//
//             std::vector<Vertex> vertices(verticesCount);
//             auto texCoords = reinterpret_cast<const float*>(fileContent);
//             auto positions = reinterpret_cast<const float*>(&fileContent[verticesBufferView["byteOffset"]]);
//             auto normals = reinterpret_cast<const float*>(&fileContent[normalsBufferView["byteOffset"]]);
//             for (int i = 0; i < verticesCount; ++i) {
//                 int offset = i * 3;
//
//                 vertices[i].pos.x = positions[offset];
//                 vertices[i].pos.y = positions[offset + 1];
//                 vertices[i].pos.z = positions[offset + 2];
//
//                 vertices[i].normal.x = normals[offset];
//                 vertices[i].normal.y = normals[offset + 1];
//                 vertices[i].normal.z = normals[offset + 2];
//
//                 offset = i * 2;
//
//                 vertices[i].texCoord.x = texCoords[offset];
//                 vertices[i].texCoord.y = texCoords[offset + 1];
//
//                 vertices[i].color = {1, 1, 1};
//             }
//
//             // file.seekg(offset);
//
//             std::vector<uint32_t> indices(indicesCount);
//             auto indicesContent = reinterpret_cast<const uint16_t*>(&fileContent[indicesBufferView["byteOffset"]]);
//             for (int i = 0; i < indicesCount; ++i) {
//                 indices[i] = indicesContent[i];
//                 // file.read(reinterpret_cast<char*>(&indices[i]), sizeof(uint16_t));
//             }
//
//             auto mesh = std::make_shared<Mesh>("", vertices, indices);
//             m_meshes.emplace_back(mesh);
//
//             return; // TODO: more meshes
//         }
//     }
// }

// Model::Model(const char *meshPath, const Texture::ID textureID) : m_textureID(textureID), m_mesh(meshPath) {
//     fmt::println("Loaded model: {} ({} vertices)", meshPath, m_mesh.getIndices().size());
// }
//
// Model::Model(Mesh mesh, const Texture::ID textureID) : m_textureID(textureID), m_mesh(std::move(mesh)) {}

Model::Model(const GLTFLoader& loader) : m_textureID(0), m_meshes(loader.meshes) {}

void Model::destroy() const {
    for (const auto& mesh : m_meshes) {
        mesh->destroy();
    }
}

const Texture::ID& Model::getTextureID() const {
    return m_textureID;
}

const std::vector<std::shared_ptr<Mesh>>& Model::getMeshes() const {
    return m_meshes;
}

// const Mesh &Model::getMesh() const {
//     return m_mesh;
// }

void Model::draw(const VkCommandBuffer& commandBuffer, const VkPipelineLayout& pipelineLayout) const {
    // for (const auto& mesh : m_meshes) {
    const std::array buffers = { m_meshes[0]->getVertexBuffer().buffer() };
    constexpr std::array<VkDeviceSize, buffers.size()> offsets = { 0 };

    vkCmdBindVertexBuffers(commandBuffer, 0, buffers.size(), buffers.data(), offsets.data());
    vkCmdBindIndexBuffer(commandBuffer, m_meshes[0]->getIndexBuffer().buffer(), 0, VK_INDEX_TYPE_UINT32);

    const glm::mat4 modelMatrix = m_transform.getMatrix();
    const glm::mat4 normalMatrix = m_transform.getNormalMatrix(modelMatrix);
    const ModelConstants constants{
        modelMatrix,
        normalMatrix,
    };

    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ModelConstants),
                       &constants);
    vkCmdDrawIndexed(commandBuffer, m_meshes[0]->getIndices().size(), 1, 0, 0, 0);
    // }
}
