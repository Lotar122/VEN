#version 450

layout(location = 0) in vec3 vc;
layout(location = 1) in vec2 tc;
layout(location = 2) in vec3 nc;
layout(location = 3) out vec3 color;

layout(location = 4) in vec4 m1;
layout(location = 5) in vec4 m2;
layout(location = 6) in vec4 m3;
layout(location = 7) in vec4 m4;

mat4 model = mat4(
    m1, m2, m3, m4
);

layout(push_constant) uniform PushConstantBlock {
    mat4 vp;
    mat4 model;
} pushConstants;

void main() {
    //magic color ;) (nihil blue) (RGB: 66, 99, 245)
    color = vec3(0.2588, 0.3882, 0.9608);

    color = vec3(tc.x, tc.y, 0.0);

    gl_Position = pushConstants.vp * model * vec4(vc, 1.0);
}