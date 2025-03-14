#version 450

layout(location = 0) in vec3 vc;
layout(location = 1) in vec2 tc;
layout(location = 2) in vec3 nc;
layout(location = 3) out vec3 color;

layout(push_constant) uniform PushConstantBlock {
    mat4 vp;
    mat4 model;
} pushConstants;

void main() {
    //magic color ;) (nihil blue) (RGB: 66, 99, 245)
    color = vec3(0.2588, 0.3882, 0.9608);

    color = vec3(0.0, tc.y, tc.x);

    gl_Position = pushConstants.vp * pushConstants.model * vec4(vc, 1.0);
}