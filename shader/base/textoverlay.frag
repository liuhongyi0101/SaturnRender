#version 450 core

layout (location = 0) in vec2 inUV;

layout (binding = 0) uniform sampler2D samplerFont;

layout (location = 0) out vec4 outFragColor;

void main(void)
{
	float color = texture(samplerFont, inUV).g;
	outFragColor = vec4(0.5,1.0,0.2, 1.0);
}
