#include "Mesh.h"

Mesh::Mesh(std::string name, std::vector<Primitive> primitives): m_name(std::move(name)),
                                                                 m_primitives(std::move(primitives)) {}

void Mesh::draw(const VkCommandBuffer& commandBuffer, const VkPipelineLayout& pipelineLayout,
                const glm::mat4& worldTransform) const {
        for (const auto& primitive : m_primitives) {
                primitive.draw(commandBuffer, pipelineLayout, worldTransform);
        }
}