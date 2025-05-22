#include "Transform.h"

glm::mat4 Transform::getNormalMatrix(const glm::mat4& modelMat) {
    const glm::mat3 model3(modelMat);
    return inverseTranspose(model3);
}

glm::mat4 Transform::getMatrix() const {
    // TODO: make model matrix member
    const glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), m_position);
    const glm::mat4 rotationMatrix = mat4_cast(m_rotation);
    const glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), m_scale);

    return translationMatrix * rotationMatrix * scaleMatrix;
}

void Transform::translate(const glm::vec3& v) {
    m_position += v;
}

void Transform::rotate(const float angle, const glm::vec3& axis) {
    m_rotation = glm::rotate(m_rotation, angle, axis);
}

void Transform::scale(const glm::vec3& v) {
    m_scale += v;
}

const glm::vec3& Transform::getPosition() const {
    return m_position;
}

void Transform::setPosition(const glm::vec3& v) {
    m_position = v;
}

const glm::quat& Transform::getRotation() const {
    return m_rotation;
}

const glm::vec3& Transform::getScale() const {
    return m_scale;
}
