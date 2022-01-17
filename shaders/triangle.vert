#version 450

void main() {
    const vec3 positions[3] = vec3[3](
        vec3(0.7f, 0.7f, 0.0f),
        vec3(-0.7f, 0.7f, 0.0f),
        vec3(0.f, -0.7f, 0.0f)
    );

    gl_Position = vec4(positions[gl_VertexIndex], 1.0f);
}