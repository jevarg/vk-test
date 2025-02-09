#pragma once

#include <glm/mat4x4.hpp>

struct alignas(16) ModelConstants {
    glm::mat4 modelMatrix;
    glm::mat4 normalMatrix;

    glm::vec3 __padding;
};