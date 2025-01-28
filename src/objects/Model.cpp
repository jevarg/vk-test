#include "Model.h"

#include <fmt/base.h>

#include <fstream>
#include <json.hpp>

using json = nlohmann::json;

// void getBufferViews(const json::basic_json& gltf, const uint64_t accessorId) {
//     const auto accessor = gltf["accessors"][accessorId];
//
// }

Model::Model(const char *filePath): m_textureID(0) {
    std::ifstream f(filePath);
    json gltf = json::parse(f);
    f.close();

    const auto fileDir = std::filesystem::path(filePath).parent_path();
    std::vector<std::ifstream> gltfBuffers;
    for (const auto& buffer : gltf["buffers"]) {
        std::ifstream bufferFile(fileDir / buffer["uri"]);
        gltfBuffers.push_back(std::move(bufferFile));
    }

    uint64_t sceneId = gltf["scene"];
    for (uint64_t nodeId : gltf["scenes"][sceneId]["nodes"]) {
        auto node = gltf["nodes"][nodeId];

        uint64_t meshId = node["mesh"];
        auto gltfMesh = gltf["meshes"][meshId];
        const std::string meshName = gltfMesh["name"];
        for (const auto& primitives : gltfMesh["primitives"]) {
            // TODO: Make the following generic
            uint64_t verticesAccessorId = primitives["attributes"]["POSITION"];
            uint64_t indicesAccessorId = primitives["indices"];

            auto verticesAccessor = gltf["accessors"][verticesAccessorId];
            auto indicesAccessor = gltf["accessors"][indicesAccessorId];

            // TODO: check "componentType" and "type"
            uint64_t verticesBufferViewId = verticesAccessor["bufferView"];
            uint64_t indicesBufferViewId = indicesAccessor["bufferView"];

            uint64_t verticesCount = verticesAccessor["count"];
            uint64_t indicesCount = indicesAccessor["count"];

            auto verticesBufferView = gltf["bufferViews"][verticesBufferViewId];
            auto indicesBufferView = gltf["bufferViews"][indicesBufferViewId];

            std::ifstream& file = gltfBuffers[verticesBufferView["buffer"]];

            int64_t offset = verticesBufferView["byteOffset"];
            file.seekg(offset);

            std::vector<Vertex> vertices(verticesCount);
            for (int i = 0; i < verticesCount; ++i) {
                file.read(reinterpret_cast<char*>(&vertices[i].pos), sizeof(glm::vec3));
            }

            offset = indicesBufferView["byteOffset"];
            file.seekg(offset);

            std::vector<uint32_t> indices(indicesCount);
            for (int i = 0; i < indicesCount; ++i) {
                file.read(reinterpret_cast<char*>(&indices[i]), sizeof(uint16_t));
            }

            auto mesh = std::make_shared<Mesh>(meshName.c_str(), vertices, indices);
            m_meshes.emplace_back(mesh);

            return; // TODO: more meshes
        }
    }
}

// Model::Model(const char *meshPath, const Texture::ID textureID) : m_textureID(textureID), m_mesh(meshPath) {
//     fmt::println("Loaded model: {} ({} vertices)", meshPath, m_mesh.getIndices().size());
// }
//
// Model::Model(Mesh mesh, const Texture::ID textureID) : m_textureID(textureID), m_mesh(std::move(mesh)) {}

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
    const std::array buffers = { m_meshes[0]->getVertexBuffer().buffer() };
    constexpr std::array<VkDeviceSize, buffers.size()> offsets = { 0 };

    vkCmdBindVertexBuffers(commandBuffer, 0, buffers.size(), buffers.data(), offsets.data());
    vkCmdBindIndexBuffer(commandBuffer, m_meshes[0]->getIndexBuffer().buffer(), 0, VK_INDEX_TYPE_UINT32);

    const glm::mat4 constants = m_transform.getMatrix();
    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4),
                       &constants);
    vkCmdDrawIndexed(commandBuffer, m_meshes[0]->getIndices().size(), 1, 0, 0, 0);
}
