#pragma once

#include <vulkan/vulkan_core.h>

#include <array>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
    glm::vec3 normal;

    [[nodiscard]]
    static const VkVertexInputBindingDescription* getBindingDescription() {
        static constexpr VkVertexInputBindingDescription desc{
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };

        return &desc;
    }

    [[nodiscard]]
    static const std::array<VkVertexInputAttributeDescription, 4>* getAttributeDescriptions() {
        static constexpr std::array<VkVertexInputAttributeDescription, 4> attrDescriptions{{
            { .location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, pos) },
            { .location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, color) },
            { .location = 2, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT,    .offset = offsetof(Vertex, texCoord) },
            { .location = 3, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, normal) },
        }};

        return &attrDescriptions;
    }
};
