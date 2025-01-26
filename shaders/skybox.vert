#version 450

layout (set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 projection;
} ubo;

layout (push_constant) uniform Constants {
    mat4 model;
} constants;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;

layout (location = 0) out vec3 fragPos;
layout (location = 1) out vec3 fragColor;

void main() {
    mat4 mvp = ubo.projection * ubo.view * constants.model;
    mat3 rot = mat3(mvp);
    vec3 pos = inPosition;

    pos.x = -pos.x;
    pos = rot * pos;

    gl_Position = pos.xyzz;
    fragPos = inPosition;
    fragColor = inColor;
}