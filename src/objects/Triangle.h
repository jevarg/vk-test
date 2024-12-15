#pragma once

#include <vector>

#include "gfx/vk/types/Vertex.h"

class Triangle {
    const std::vector<Vertex> m_vertices{
        {  { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
        {   { 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
        {    { 0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
        {   { -0.5f, 0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },

        { { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
        {  { 0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
        {   { 0.5f, 0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
        {  { -0.5f, 0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
    };

    const std::vector<uint16_t> m_indices{
        0, 1, 2, 2, 3, 0,  //
        4, 5, 6, 6, 7, 4,  //
    };

   public:
    [[nodiscard]]
    size_t getByteSize() const;

    [[nodiscard]]
    size_t getIndicesByteSize() const;

    [[nodiscard]]
    const std::vector<Vertex>& getVertices() const;

    [[nodiscard]]
    const std::vector<uint16_t>& getIndices() const;
};
