#version 450

layout(location = 0) out vec4 FragColor;  // Output color
layout(location = 3) in vec3 color;

void main() {
    FragColor = vec4(color, 1.0);
}
