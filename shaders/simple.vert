#version 450

#define M_PI 3.1415926535897932384626433832795

layout (set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 projection;
} ubo;

layout (push_constant) uniform Constants {
    mat4 modelMatrix;
    mat4 normalMatrix;
} constants;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 inNormal;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec2 fragTexCoord;
layout (location = 2) out vec3 fragNormal;
layout (location = 3) out vec3 fragPosition;
layout (location = 4) out vec3 fragView;

void main() {
    gl_Position = ubo.projection * ubo.view * constants.modelMatrix * vec4(inPosition, 1.0);

    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragNormal = mat3(constants.normalMatrix) * inNormal;
    fragPosition = inPosition;
    fragView = (ubo.projection * ubo.view)[2].xyz; // view dir
}