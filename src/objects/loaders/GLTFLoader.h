#pragma once

#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <json.hpp>
#include <vector>

#include "GLTF.h"
#include "objects/Mesh.h"
#include "objects/Primitive.h"

class Model;

class GLTFLoader {
    struct Files {
        std::vector<std::vector<uint8_t>> buffers;
        std::vector<std::vector<uint8_t>> images;
    };

public:
    explicit GLTFLoader(const char* filePath);

    std::unique_ptr<Model> acquire();

    // std::vector<std::shared_ptr<Mesh>> meshes;
    // std::vector<std::shared_ptr<Materials>> materials;

private:
    void _loadFiles(const std::filesystem::path& rootPath);

    Mesh _buildMesh(uint64_t meshId) const;
    GLTF::Primitive _getPrimitiveBuffer(const nlohmann::json& primitive, const char* key) const;
    GLTF::Material _getMaterial(uint64_t materialId) const;
    // void loadVertices();

    std::string m_filePath;
    nlohmann::json m_gltf;
    Files m_files;
    std::unique_ptr<Model> m_model;
};

