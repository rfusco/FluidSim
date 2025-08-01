#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 instancePos;
layout(location = 2) in float instanceRadius;
layout(location = 3) in vec3 instanceColor;

uniform mat4 u_projection;

out vec2 fragLocalPos;
out vec3 fragColor;

void main() {
    fragLocalPos = aPos;
    vec2 scaled = aPos * instanceRadius;
    vec2 worldPos = scaled + instancePos;
    gl_Position = u_projection * vec4(worldPos, 0.0, 1.0);
    fragColor = instanceColor;
}
