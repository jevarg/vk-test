#pragma once

#include <SDL_mouse.h>

#include <glm/vec2.hpp>

class Mouse {
   public:
    static void update() {
        if (!SDL_GetRelativeMouseMode()) {
            m_delta.x = 0;
            m_delta.y = 0;
            return;
        }

        int rawMouseX = 0;
        int rawMouseY = 0;

        uint32_t mMouseButtonState = SDL_GetRelativeMouseState(&rawMouseX, &rawMouseY);
        m_delta.x = static_cast<float>(rawMouseX) / 1280;
        m_delta.y = static_cast<float>(rawMouseY) / 1024;
    }

    static const glm::vec2& getDelta() {
        return m_delta;
    }

    // static void reset() {
    //     m_delta.x = 0;
    //     m_delta.y = 0;
    // }

   private:
    inline static glm::vec2 m_delta{};
};