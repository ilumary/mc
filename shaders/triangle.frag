#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;

layout (location = 0) out vec4 outFragColor;

const vec3 lightPos = vec3(10.0, 10.0, 0.0);

void main() {
    vec3 baseColor = vec3(0.4, 0.9, 0.4);
    vec3 ambient = 0.05 * baseColor;

    vec3 lightDir = normalize(lightPos - inPos);
    vec3 normal = normalize(inNormal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * ambient;

    outFragColor = vec4(ambient + diffuse, 1.f);
}