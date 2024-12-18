#pragma once

#include <SDL2/SDL_keyboard.h>

class Keyboard {
   public:
    static void init() {
        m_keyboardState = SDL_GetKeyboardState(nullptr);
    }

    static void update() {
        m_keyboardModState = SDL_GetModState();
    }

    static bool isKeyPressed(const SDL_Scancode code) {
        return m_keyboardState[code] == 1;
    }

    static bool isModPressed(const SDL_Keymod mod) {
        return (m_keyboardModState & mod) != 0;
    }

   private:
    inline static const uint8_t* m_keyboardState = nullptr;
    inline static SDL_Keymod m_keyboardModState = KMOD_NONE;
};
