#version 450

#extension GL_KHR_vulkan_glsl : enable

// Vertex position for a fullscreen quad
const vec2 vertices[4] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0,  1.0)
);

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 colorInput;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 normal;
layout(location = 4) out vec4 colorInt;

layout(location = 5) in vec4 instance1x4;
layout(location = 6) in vec4 instance2x4;
layout(location = 7) in vec4 instance3x4;
layout(location = 8) in vec4 instance4x4;

layout(push_constant) uniform constants {
    mat4 pre;
    mat4 proj;
    mat4 view;
    mat4 model;
} ObjectData;

bool quad = false;

void main()
{
    mat4 instanceMatrix = mat4(
        instance1x4,
        instance2x4,
        instance3x4,
        instance4x4
    );

    //instanceMatrix behaves as model matrix
	gl_Position = ObjectData.proj * ObjectData.view * instanceMatrix * vec4(vertexPosition, 1.0);
	colorInt = vec4(colorInput, 1.0);

    if(quad)
    {
        vec2 vertexPos;
        // Get vertex position for this invocation
        if(!(gl_VertexIndex >= 4))
        {vertexPos = vertices[gl_VertexIndex];}
        else {vertexPos = vec2(0);}

        // Output position (in normalized device coordinates)
        if(vertexPosition == vec3(1.0f))
        {
        colorInt = vec4(1.0f, 0.0f, 0.0f, 1.0f);
        }
        else 
        {
        colorInt = vec4(0.0f, 1.0f, 0.0f, 1.0f);
        }
        gl_Position = vec4(vertexPos, 0.0, 1.0);
    }
}