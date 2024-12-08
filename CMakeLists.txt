cmake_minimum_required(VERSION 3.29)
project(VKTest)

set(CMAKE_CXX_STANDARD 20)
set(VULKAN_SDK $ENV{VULKAN_SDK})

find_package(fmt REQUIRED)
find_package(SDL2 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)

find_library(SHADERC_COMBINED_LIB
        NAMES shaderc_combinedd
        HINTS ${VULKAN_SDK}/Lib
        REQUIRED
)

find_library(SHADERC_UTIL_LIB
        NAMES shaderc_utild
        HINTS ${VULKAN_SDK}/Lib
        REQUIRED
)

add_executable(VKTest
        src/main.cpp
        src/vkutil.h

        src/OneTimeCommand.cpp
        src/OneTimeCommand.h
        src/VK.cpp
        src/VK.h

        src/gpu_resources/Buffer.cpp
        src/gpu_resources/Buffer.h
        src/gpu_resources/DepthImage.cpp
        src/gpu_resources/DepthImage.h
        src/gpu_resources/Image.cpp
        src/gpu_resources/Image.h
        src/gpu_resources/Shader.cpp
        src/gpu_resources/Shader.h
        src/gpu_resources/Texture.cpp
        src/gpu_resources/Texture.h

        src/objects/Model.cpp
        src/objects/Model.h
        src/objects/Triangle.h
        src/objects/Triangle.cpp

        src/types/Vertex.h
        src/types/UniformBufferObject.h
        src/types/VulkanContext.h
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
        ${SHADERC_COMBINED_LIB}
        ${SHADERC_UTIL_LIB}
)
