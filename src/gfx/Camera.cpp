#include "Camera.h"

#include <glm/ext/matrix_clip_space.hpp>

#include "input/Keyboard.h"
#include "input/Mouse.h"

Camera::Camera(const float aspectRatio) {
    m_projection = glm::perspective(glm::radians(60.0f), aspectRatio, 0.1f, 10.0f);
}

glm::mat4 Camera::getView() const {
    const glm::vec3 fwdVec = m_transform.rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    return lookAt(m_transform.position, m_transform.position + fwdVec, glm::vec3(0.0f, 1.0f, 0.0f));
}

const glm::mat4& Camera::getProjection() const {
    return m_projection;
}

void Camera::update(const float delta) {
    if (Keyboard::isKeyPressed(SDL_SCANCODE_W)) {
        localTranslate({ 0.0f, 0.0f, -m_speed });
    }

    if (Keyboard::isKeyPressed(SDL_SCANCODE_S)) {
        localTranslate({ 0.0f, 0.0f, m_speed });
    }

    if (Keyboard::isKeyPressed(SDL_SCANCODE_A)) {
        localTranslate({ -m_speed, 0.0f, 0.0f });
    }

    if (Keyboard::isKeyPressed(SDL_SCANCODE_D)) {
        localTranslate({ m_speed, 0.0f, 0.0f });
    }

    const glm::vec2& mouseDelta = Mouse::getDelta();
    if (mouseDelta.x) {
        rotate(-m_sensitivity * mouseDelta.x, { 0.0f, 1.0f, 0.0f });
    }

    if (mouseDelta.y) {
        rotate(-m_sensitivity * mouseDelta.y, { 1.0f, 0.0f, 0.0f });
    }
}
