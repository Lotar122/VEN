#version 450

layout(location = 0) in vec3 vc;
layout(location = 1) in vec2 tc;
layout(location = 2) in vec3 nc;
layout(location = 3) out vec3 color;
layout(location = 4) out vec3 vNormal;
layout(location = 5) out vec3 vPosition;

layout(push_constant) uniform PushConstantBlock {
    mat4 vp;
    mat4 model;
} pushConstants;

void main() {
    //magic color ;) (nihil blue) (RGB: 66, 99, 245)
    color = vec3(0.2588, 0.3882, 0.9608);

    vNormal = mat3(pushConstants.model) * nc;
    vPosition = (pushConstants.model * vec4(vc, 1.0f)).xyz;

    gl_Position = pushConstants.vp * pushConstants.model * vec4(vc, 1.0);
}