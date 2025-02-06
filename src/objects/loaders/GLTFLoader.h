#pragma once

#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <json.hpp>
#include <vector>

#include "GLTF.h"
#include "objects/Mesh.h"

template <typename T>
glm::vec3 make_any(std::any data) {
    return glm::make_vec3<T>(static_cast<T>(data));
}

class GLTFLoader {
    struct Files {
        std::vector<std::vector<uint8_t>> buffers;
        std::vector<std::vector<uint8_t>> images;
    };

   public:
    explicit GLTFLoader(const char* filePath);

    std::vector<std::shared_ptr<Mesh>> meshes;
    // std::vector<std::shared_ptr<Materials>> materials;

   private:
    void loadFiles(const std::filesystem::path& rootPath);
    GLTF::Primitive getPrimitiveBuffer(const nlohmann::json& primitive, const char* key);
    // void loadVertices();

    nlohmann::json m_gltf;
    Files m_files;
};

