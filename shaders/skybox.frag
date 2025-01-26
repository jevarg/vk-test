#version 450

layout (set = 1, binding = 0) uniform samplerCube texSampler;

layout (location = 0) in vec3 fragPos;
layout (location = 1) in vec3 fragColor;

layout (location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, fragPos) * vec4(fragColor, 1);
}