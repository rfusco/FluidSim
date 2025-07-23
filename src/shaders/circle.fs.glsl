#version 330 core

in vec2 fragLocalPos;
out vec4 FragColor;

void main() {
    float dist = length(fragLocalPos);

    if (dist > 1.0) {
        discard; // Don't render outside the unit circle
    }

    float alpha = smoothstep(1.0, 0.95, dist);
    FragColor = vec4(1.0, 1.0, 1.0, alpha);
}
