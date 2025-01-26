#include <SDL2/SDL.h>
#include <fmt/base.h>

#include "gfx/vk/VK.h"

void PrintSDLError() {
    fmt::println(stderr, "Error: {}", SDL_GetError());
}

void DestroySDL(SDL_Window* window) {
    if (window != nullptr) {
        SDL_DestroyWindow(window);
    }

    SDL_Quit();
    fmt::println("SDL Destroyed");
}

SDL_Window* InitSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        PrintSDLError();
        return nullptr;
    }

    SDL_Window* window = SDL_CreateWindow("VKTest", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 1024,
                                          SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (window == nullptr || SDL_SetRelativeMouseMode(SDL_TRUE) != 0) {
        PrintSDLError();
        DestroySDL(window);
        return nullptr;
    }

    fmt::println("SDL Initialized");
    return window;
}

int main(int, char**) {
#ifndef NDEBUG
    fmt::println("=== THIS IS A DEBUG BUILD ===");
#endif

    SDL_Window* window = InitSDL();
    if (window == nullptr) {
        return EXIT_FAILURE;
    }

    try {
        VK app(window);
        app.run();
    } catch (const std::exception& e) {
        fmt::println(stderr, "App error: {}", e.what());
    }

    DestroySDL(window);
    return EXIT_SUCCESS;
}
