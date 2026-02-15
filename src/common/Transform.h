#pragma once

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class Transform {
public:
    [[nodiscard]]
    static glm::mat4 getNormalMatrix(const glm::mat4& modelMat);

    [[nodiscard]]
    glm::mat4 getMatrix() const;

    void translate(const glm::vec3& v);
    void translateLocal(const glm::vec3& v);

    void rotate(float angle, const glm::vec3& axis);
    void rotateLocal(float angle, const glm::vec3& axis);

    void scale(const glm::vec3& v);

    [[nodiscard]]
    const glm::vec3& getPosition() const;
    void setPosition(const glm::vec3& v);

    [[nodiscard]]
    const glm::quat& getRotation() const;

    [[nodiscard]]
    const glm::vec3& getScale() const;

private:
    // glm::mat4 m_matrix = glm::identity<glm::mat4>();
    glm::vec3 m_position = glm::vec3(0.0f);
    glm::quat m_rotation = glm::identity<glm::quat>();
    glm::vec3 m_scale = glm::vec3(1.0f);
};
