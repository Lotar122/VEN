#version 450

layout(location = 0) out vec4 FragColor;  // Output color
layout(location = 3) in vec3 color;
layout(location = 8) in vec3 vNormal;
layout(location = 9) in vec3 vPosition;

const vec3 lightPosition = vec3(0.0f, -10.0f, -20.0f);
const vec3 lightColor = vec3(1.0f);

void main() {
    vec3 lightDir = lightPosition - vPosition;

    vec3 nNormalized = normalize(vNormal);
    vec3 lNormalized = normalize(lightDir);

    vec3 diffuse = lightColor * max(vec3(0), dot(lNormalized, nNormalized));
    
    vec3 result = diffuse * color;
    FragColor = vec4(result, 1.0);
}
