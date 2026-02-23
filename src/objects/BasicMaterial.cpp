#include "BasicMaterial.h"

#include <utility>

BasicMaterial::BasicMaterial(std::string name, Pipeline::Type pipelineType, const Handle<Texture> texture)
    : m_name(std::move(name)), m_pipelineType(pipelineType), m_texture(texture) {}

Pipeline::Type BasicMaterial::getPipelineType() const {
    return m_pipelineType;
}

Handle<Texture> BasicMaterial::getTexture() const {
    return m_texture;
}
