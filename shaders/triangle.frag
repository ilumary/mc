#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTextCord;

layout (location = 0) out vec4 outFragColor;

const vec3 lightPos = vec3(0.0, -20.0, 0.0);

void main() {
    vec3 baseColor = vec3(1, 0.95, 0.7);
    vec3 ambient = 0.05 * baseColor;

    vec3 lightDir = normalize(lightPos - inPos);
    vec3 normal = normalize(inNormal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * ambient;

    outFragColor = texture(texSampler, inTextCord);
    outFragColor.rgb *= (ambient + diffuse);
}