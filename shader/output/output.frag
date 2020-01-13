#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform sampler2D sceneColor;
layout (binding = 1) uniform UBO 
{
   float count;
} ubo;
layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outFragColor;


void main() 
{

    vec4 color =  texture(sceneColor, inUV)/ubo.count;

    color.rgb = pow(color.rgb,vec3(0.45));
        outFragColor =color; 
}

