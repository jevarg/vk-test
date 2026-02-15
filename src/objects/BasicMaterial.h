#pragma once

#include <string>

#include "gfx/Handle.h"
#include "gfx/vk/pipeline/Pipeline.h"

class Pipeline;
class Texture;
class BasicMaterial {
public:
    BasicMaterial(std::string name, Pipeline::Type pipelineType, Handle<Texture> texture);
    BasicMaterial(BasicMaterial&& other) noexcept = default;

    [[nodiscard]]
    Pipeline::Type getPipelineType() const;

    [[nodiscard]]
    Handle<Texture> getTexture() const;

   private:
    std::string m_name;

    [[deprecated]]
    Pipeline::Type m_pipelineType;
    Handle<Texture> m_texture;
};
