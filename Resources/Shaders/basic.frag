#version 450

layout(location = 0) out vec4 FragColor;  // Output color
layout(location = 3) in vec2 tCoord;
layout(location = 4) in vec3 vNormal;
layout(location = 5) in vec3 vPosition;

const vec3 lightPosition = vec3(0.0f, -10.0f, -20.0f);
const vec3 lightColor = vec3(1.0f);

layout(set = 0, binding = 0) uniform texture2D uTexture;
layout(set = 0, binding = 1) uniform sampler uSampler;

void main() {
    vec3 lightDir = lightPosition - vPosition;

    vec3 nNormalized = normalize(vNormal);
    vec3 lNormalized = normalize(lightDir);

    vec3 diffuse = lightColor * max(vec3(0), dot(lNormalized, nNormalized));

    vec4 textureColor = texture(sampler2D(uTexture, uSampler), tCoord * -1.0f);
    if(textureColor.w == 0.0f) textureColor = vec4(0.2588, 0.3882, 0.9608, 1.0);
    
    vec4 result = vec4(diffuse, 1.0f) * (textureColor * 1.2);
    FragColor = result;
}
