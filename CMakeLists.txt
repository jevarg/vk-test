cmake_minimum_required(VERSION 3.29)
project(VKTest)

set(CMAKE_CXX_STANDARD 20)

find_package(fmt REQUIRED)
find_package(SDL2 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(VKTest
        src/main.cpp
        src/Buffer.cpp
        src/Buffer.h
        src/OneTimeCommand.cpp
        src/OneTimeCommand.h
        src/Shader.cpp
        src/Shader.h
        src/Texture.cpp
        src/Texture.h
        src/VK.cpp
        src/VK.h
        src/vkutil.h

        src/types/Vertex.h
        src/types/UniformBufferObject.h

        src/objects/Triangle.h
        src/objects/Triangle.cpp
)

target_include_directories(VKTest PRIVATE
        src
        external/include
)
target_link_libraries(VKTest PRIVATE
        fmt::fmt
        SDL2::SDL2
        SDL2::SDL2main
        glm::glm
        Vulkan::Vulkan
)
