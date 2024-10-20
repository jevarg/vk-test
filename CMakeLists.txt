cmake_minimum_required(VERSION 3.29)
project(VKTest)

set(CMAKE_CXX_STANDARD 20)

find_package(fmt REQUIRED)
find_package(SDL2 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(VKTest
        src/main.cpp
        src/VK.cpp
        src/VK.h
        src/Shader.cpp
        src/Shader.h
)

target_link_libraries(VKTest PRIVATE
        fmt::fmt
        SDL2::SDL2
        SDL2::SDL2main
        glm::glm
        Vulkan::Vulkan
)
