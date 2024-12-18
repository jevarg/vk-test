#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

#include "common/Thing.h"
#include "common/Transform.h"

class Camera : public Thing {
   public:
    explicit Camera(float aspectRatio);

    [[nodiscard]]
    glm::mat4 getView() const;

    [[nodiscard]]
    const glm::mat4& getProjection() const;

    void update(float delta);

   private:
    glm::mat4 m_projection{};

    float m_speed = 0.1f;
    float m_sensitivity = 0.5f;
};
