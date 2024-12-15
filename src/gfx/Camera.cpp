#include "Camera.h"

#include <glm/ext/matrix_clip_space.hpp>

Camera::Camera(const float aspectRatio) {
    m_projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 10.0f);
};

void Camera::setPosition(const glm::vec3& pos) {
    m_pos = pos;
    m_hasMoved = true;
}

const glm::mat4& Camera::getView() {
    if (m_hasMoved) {
        m_view = lookAt(m_pos, m_target, glm::vec3(0.0f, 0.0f, 1.0f));
        m_hasMoved = false;
    }

    return m_view;
}

const glm::mat4& Camera::getProjection() const {
    return m_projection;
}

void Camera::translate(const glm::vec3& vec) {
    setPosition(m_pos + vec);
}
