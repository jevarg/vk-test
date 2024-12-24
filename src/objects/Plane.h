#pragma once
#include "Model.h"

class Plane : public Model {
public:
    explicit Plane(const VulkanContext& vkContext, const char* texturePath);
};
