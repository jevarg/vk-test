#pragma once

#include <SDL_mouse.h>

#include <glm/vec2.hpp>

class Mouse {
   public:
    static void update() {
        int rawMouseX = 0;
        int rawMouseY = 0;

        uint32_t mMouseButtonState = SDL_GetRelativeMouseState(&rawMouseX, &rawMouseY);
        m_delta.x = static_cast<float>(rawMouseX) / 640;
        m_delta.y = static_cast<float>(rawMouseY) / 480;
    }

    static const glm::vec2& getDelta() {
        return m_delta;
    }

   private:
    inline static glm::vec2 m_delta{};
};