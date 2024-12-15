#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

class Camera {
   public:
    explicit Camera(float aspectRatio);

    void setPosition(const glm::vec3& pos);

    [[nodiscard]]
    const glm::mat4& getView();

    [[nodiscard]]
    const glm::mat4& getProjection() const;

    void translate(const glm::vec3& vec);

   private:
    glm::vec3 m_pos{};
    glm::vec3 m_target{};
    glm::mat4 m_projection{};

    bool m_hasMoved = true;
    glm::mat4 m_view = glm::identity<glm::mat4>();
};
