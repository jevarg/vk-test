#pragma once
#include <string>

#include "Primitive.h"

class Mesh {
public:
    Mesh(std::string name, std::vector<Primitive> primitives);

private:
    std::string m_name;
    std::vector<Primitive> m_primitives;
};
