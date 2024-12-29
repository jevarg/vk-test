#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

struct Transform {
    glm::vec3 position = glm::vec3(0.0f);
    glm::quat rotation = glm::identity<glm::quat>();
    glm::vec3 scale = glm::vec3(1.0f);

    [[nodiscard]]
    glm::mat4 getMatrix() const {
        const glm::mat4 translationMatrix = translate(glm::mat4(1.0f), position);
        const glm::mat4 rotationMatrix = mat4_cast(rotation);
        const glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

        return translationMatrix * rotationMatrix * scaleMatrix;
    }

    void rotate(const float angle, const glm::vec3& axis) {
        rotation = glm::rotate(rotation, angle, axis);
    }
};
