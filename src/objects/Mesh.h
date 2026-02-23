#pragma once

#include <string>
#include <vector>

#include "BasicMaterial.h"
#include "gfx/vk/gpu_resources/Buffer.h"
#include "gfx/vk/types/Vertex.h"

struct Submesh {
    uint32_t indexOffset = 0;
    uint32_t indexCount = 0;
    uint32_t materialIndex = 0;
};

class Mesh {
public:
    Mesh(std::string name, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
         const std::vector<Handle<BasicMaterial>>& materials);
    Mesh(std::string name, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
         const std::vector<Handle<BasicMaterial>>& materials, const std::vector<Submesh>& submeshes);

    [[nodiscard]]
    const Buffer& getVertexBuffer() const;

    [[nodiscard]]
    const Buffer& getIndexBuffer() const;

    [[nodiscard]]
    const std::vector<Submesh>& getSubmeshes() const;

    [[nodiscard]]
    const std::vector<Handle<BasicMaterial>>& getMaterials() const;

private:
    [[nodiscard]]
    Buffer m_createVertexBuffer(const std::vector<Vertex>& vertices) const;

    [[nodiscard]]
    Buffer m_createIndexBuffer(const std::vector<uint32_t>& indices) const;

    std::string m_name;

    Buffer m_vertexBuffer;
    Buffer m_indexBuffer;

    std::vector<Handle<BasicMaterial>> m_materials;
    std::vector<Submesh> m_submeshes;
};
