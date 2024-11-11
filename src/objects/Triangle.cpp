#include "Triangle.h"

size_t Triangle::getByteSize() const {
    return sizeof(m_vertices[0]) * m_vertices.size();
}

size_t Triangle::getIndicesByteSize() const {
    return sizeof(m_indices[0]) * m_indices.size();
}

const std::vector<Vertex>& Triangle::getVertices() const {
    return m_vertices;
}

const std::vector<uint16_t>& Triangle::getIndices() const {
    return m_indices;
}
