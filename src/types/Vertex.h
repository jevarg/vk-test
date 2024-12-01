#pragma once

#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription desc{};
        desc.binding = 0;
        desc.stride = sizeof(Vertex);
        desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return desc;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attrDescriptions{};
        attrDescriptions[0].binding = 0;
        attrDescriptions[0].location = 0;
        attrDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attrDescriptions[0].offset = offsetof(Vertex, pos);

        attrDescriptions[1].binding = 0;
        attrDescriptions[1].location = 1;
        attrDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attrDescriptions[1].offset = offsetof(Vertex, color);

        attrDescriptions[2].binding = 0;
        attrDescriptions[2].location = 2;
        attrDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attrDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attrDescriptions;
    }
};
