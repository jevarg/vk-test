cmake_minimum_required(VERSION 3.29)
project(VKTest)

set(CMAKE_CXX_STANDARD 20)
set(VULKAN_SDK $ENV{VULKAN_SDK})

find_package(fmt REQUIRED)
find_package(SDL2 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)

set(SHADERC_COMBINED_LIB_NAME "shaderc_combined")
set(SHADERC_UTIL_LIB_NAME "shaderc_util")

if (WIN AND DEBUG)
    set(SHADERC_COMBINED_LIB_NAME "{$SHADERC_COMBINED_LIB_NAME}d")
    set(SHADERC_UTIL_LIB_NAME "{$SHADERC_UTIL_LIB_NAME}d")
endif ()

find_library(SHADERC_COMBINED_LIB
        NAMES ${SHADERC_COMBINED_LIB_NAME}
        HINTS ${VULKAN_SDK}/Lib
        REQUIRED
)

find_library(SHADERC_UTIL_LIB
        NAMES ${SHADERC_UTIL_LIB_NAME}
        HINTS ${VULKAN_SDK}/Lib
        REQUIRED
)

add_executable(VKTest
        src/main.cpp

        src/gfx/Camera.cpp
        src/gfx/Camera.h
        src/gfx/vk/OneTimeCommand.cpp
        src/gfx/vk/OneTimeCommand.h
        src/gfx/vk/VK.cpp
        src/gfx/vk/VK.h
        src/gfx/vk/vkutil.h
        src/gfx/vk/gpu_resources/Buffer.cpp
        src/gfx/vk/gpu_resources/Buffer.h
        src/gfx/vk/gpu_resources/DepthImage.cpp
        src/gfx/vk/gpu_resources/DepthImage.h
        src/gfx/vk/gpu_resources/Image.cpp
        src/gfx/vk/gpu_resources/Image.h
        src/gfx/vk/gpu_resources/PhysicalDevice.cpp
        src/gfx/vk/gpu_resources/PhysicalDevice.h
        src/gfx/vk/gpu_resources/Shader.cpp
        src/gfx/vk/gpu_resources/Shader.h
        src/gfx/vk/gpu_resources/Texture.cpp
        src/gfx/vk/gpu_resources/Texture.h
        src/gfx/vk/pipeline/Pipeline.cpp
        src/gfx/vk/pipeline/Pipeline.h
        src/gfx/vk/types/UniformBufferObject.h
        src/gfx/vk/types/Vertex.h
        src/gfx/vk/types/VulkanContext.h
        src/gfx/vk/types/VulkanContext.cpp

        src/objects/Model.cpp
        src/objects/Model.h
        src/objects/Triangle.h
        src/objects/Triangle.cpp
        src/objects/prefabs/Plane.cpp
        src/objects/prefabs/Plane.h
        src/objects/Mesh.cpp
        src/objects/Mesh.h
        src/objects/loaders/GLTFLoader.cpp
        src/objects/loaders/GLTFLoader.h
        src/common/Transform.h
        src/common/Thing.h
        src/input/Keyboard.h
        src/input/Mouse.h
        src/objects/prefabs/Cube.cpp
        src/objects/prefabs/Cube.h
        src/objects/loaders/GLTF.h
        src/gfx/vk/types/ModelConstants.h
        src/objects/Material.cpp
        src/objects/Material.h
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
