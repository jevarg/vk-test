#pragma once

#include <string>

#include "Primitive.h"

class Mesh {
public:
    Mesh(std::string name, std::vector<Primitive> primitives);

    void draw(const VkCommandBuffer& commandBuffer, const VkPipelineLayout& pipelineLayout,
              const glm::mat4& worldTransform) const;

private:
    std::string m_name;
    std::vector<Primitive> m_primitives;
};
