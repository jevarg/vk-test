#pragma once

#include <vector>
#include <types/Vertex.h>

class Triangle {
    const std::vector<Vertex> m_vertices = {
        {{0.2f, -0.9f}, {1.0f, 0.4f, 0.0f}},
        {{0.5f, 0.6f}, {0.0f, 1.0f, 1.0f}},
        {{-0.5f, 1.0f}, {0.0f, 0.1f, 1.0f}}
    };

public:
    [[nodiscard]]
    size_t getByteSize() const;

    [[nodiscard]]
    const std::vector<Vertex>& getVertices() const;
};
