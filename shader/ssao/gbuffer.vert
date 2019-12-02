#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 model;
	mat4 view;
	mat4 lightSpace;
} ubo;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outPos;
layout (location = 3) out vec4 outShadowCoord;
out gl_PerVertex
{
	vec4 gl_Position;
};
const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );
void main() 
{
	gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPos,1.0);
	
	outUV = inUV;

	// Vertex position in view space
	outPos = vec3(ubo.view * ubo.model * vec4(inPos,1.0));

	// Normal in view space
	mat3 normalMatrix = transpose(inverse(mat3(ubo.view * ubo.model)));
	outNormal = normalMatrix * inNormal;
	outShadowCoord = ( biasMat * ubo.lightSpace  * ubo.model) * vec4(inPos, 1.0);	

}
