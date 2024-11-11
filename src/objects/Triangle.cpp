#include "Triangle.h"

size_t Triangle::getByteSize() const {
    return sizeof(m_vertices[0]) * m_vertices.size();
}

const std::vector<Vertex>& Triangle::getVertices() const {
    return m_vertices;
}
