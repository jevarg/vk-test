#include "Mesh.h"

Mesh::Mesh(std::string name, std::vector<Primitive> primitives): m_name(std::move(name)),
                                                                 m_primitives(std::move(primitives)) {}