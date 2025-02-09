#version 450

layout (set = 1, binding = 0) uniform sampler2D texSampler;

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec2 fragTexCoord;
layout (location = 2) in vec3 fragNormal;
layout (location = 3) in vec3 fragPos;
layout (location = 4) in vec3 fragView;

layout (location = 0) out vec4 outColor;

const vec3 lightPos = vec3(-5.0, 0.0, 5.0);
const vec3 lightColor = vec3(1.0, 1.0, 1.0);

const float ambientStrength = 0.1;
const float K = 1.0;

//vec3 phong(vec3 p) {
//    return vec3(1.0, 1.0, 1.0);
//}

void main() {
    vec3 normal = normalize(fragNormal);
    vec4 texColor = texture(texSampler, fragTexCoord);
    vec3 lightDir = normalize(lightPos - fragPos);
    vec4 ambient = vec4(ambientStrength * lightColor, 1.0);
    float diffuse = max(dot(normal, lightDir), 0.0);

    outColor = (ambient + diffuse) * texColor;
//    outColor = ambient * texColor;

//    outColor = vec4(1, 1, 1, 1);
//    outColor *= vec4(I, I, I, 1);
}