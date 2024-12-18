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

    virtual void setPosition(const glm::vec3& v) {
        m_transform.position = v;
    }

   protected:
    Transform m_transform;
};