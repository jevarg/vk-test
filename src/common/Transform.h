#pragma once

#include <glm/ext/matrix_transform.hpp>

struct Transform {
    glm::mat4 translation = glm::mat4(1.0f);
    glm::mat4 rotation = glm::mat4(1.0f);
    glm::mat4 scale = glm::mat4(1.0f);

    glm::mat4 getMatrix() const {
        return translation * rotation * scale;
    }

    void translate(const glm::vec3& v) {
        translation = glm::translate(translation, v);
    }

    void rotate(const glm::vec4& v) {
        rotation = glm::rotate(rotation, v.w, glm::vec3(v));
    }
};
