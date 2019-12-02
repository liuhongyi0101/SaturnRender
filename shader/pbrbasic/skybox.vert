#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;

layout (binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 model;      
} ubo;

layout (location = 0) out vec3 outUVW;

out gl_PerVertex 
{
	vec4 gl_Position;
};

void main() 
{
	outUVW = inPos;
	mat4 view  = ubo.model;
	//view[3] = vec4(vec3(0.0),0.0);
	outUVW.x *= -1.0;
	gl_Position = ubo.projection * view * vec4(inPos.xyz, 1.0);
	gl_Position.z = gl_Position.w;
	
}
