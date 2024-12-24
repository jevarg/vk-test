#pragma once

#include "Transform.h"

class Thing {
   public:
    virtual ~Thing() = default;

    virtual void translate(const glm::vec3& v) {
        m_transform.position += v;
    }

    virtual void localTranslate(const glm::vec3& v) {
        m_transform.position += m_transform.rotation * v;
    }

    virtual void rotate(const float angle, const glm::vec3& v) {
        m_transform.rotate(angle, v);
    }

    virtual void scale(const glm::vec3& v) {
        m_transform.scale += v;
    }

    virtual void setPosition(const glm::vec3& v) {
        m_transform.position = v;
    }

    [[nodiscard]]
    const Transform& getTransform() const {
        return m_transform;
    }

   protected:
    Transform m_transform;
};