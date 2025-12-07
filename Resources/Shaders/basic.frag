#version 450

layout(location = 0) out vec4 FragColor;  // Output color
layout(location = 3) in vec2 tCoord;
layout(location = 4) in vec3 vNormal;
layout(location = 5) in vec3 vPosition;

layout(set = 0, binding = 0) uniform texture2D uTexture;
layout(set = 0, binding = 1) uniform sampler uSampler;
layout(std430, set = 0, binding = 2) readonly buffer LightInfo {
    vec4 lightColor;
    vec4 lightDir;
    vec4 lightPos;
    float lightLength;
    float radius;
} lightInfo;

float projectOntoAxis(vec4 p, vec4 axisOrigin, vec4 axisDir)
{
    return dot(p - axisOrigin, axisDir);
}

float distanceToAxis(vec4 p, vec4 axisOrigin, vec4 axisDir)
{
    vec4 rel = p - axisOrigin;
    float t = dot(rel, axisDir);
    vec4 closest = axisOrigin + t * axisDir;
    return length(p - closest);
}

vec4 calculateCylinderLight(vec4 fragPos, vec4 normal, vec4 viewDir)
{
    float axisDist = projectOntoAxis(fragPos, lightInfo.lightPos, lightInfo.lightDir);
    float radialDist = distanceToAxis(fragPos, lightInfo.lightPos, lightInfo.lightDir);

    if(axisDist < 0.0f || axisDist > lightInfo.lightLength) return vec4(0.0f);
    if(radialDist > lightInfo.radius) return vec4(0.0f);

    vec4 L = normalize(-lightInfo.lightDir);

    float diff = max(dot(normal, L), 0.0f);

    vec4 H = normalize(L + viewDir);

    //specular in the future

    float radialFallof = 1.0f - smoothstep(lightInfo.radius * 0.8f, lightInfo.radius, radialDist);

    float axialFallof = 1.0f - smoothstep(lightInfo.lightLength * 0.8f, lightInfo.lightLength, axisDist);

    float attenuation = radialFallof * axialFallof;

    return lightInfo.lightColor * diff * attenuation;
}

void main() {
    vec3 nNormalized = normalize(vNormal);
    vec3 lNormalized = normalize(vec3(lightInfo.lightDir));

    float axisDist = projectOntoAxis(vec4(vPosition, 1.0f), lightInfo.lightPos, lightInfo.lightDir);
    float axialFallof = 1.0f - smoothstep(lightInfo.lightLength * 0.8f, lightInfo.lightLength, axisDist);

    vec4 diffuse = lightInfo.lightColor * vec4(max(vec3(0), dot(lNormalized, nNormalized)), 1.0f) * axialFallof;

    vec4 textureColor = texture(sampler2D(uTexture, uSampler), tCoord * -1.0f);
    if(textureColor.w == 0.0f) textureColor = vec4(0.2588, 0.3882, 0.9608, 1.0);
    
    vec4 result = diffuse * (textureColor * 1.2);
    FragColor = result;
}
