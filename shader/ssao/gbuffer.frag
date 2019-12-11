#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inPos;
layout (location = 3) in vec4 inShadowCoord;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;
layout (location = 3) out vec4 outmix;
const float NEAR_PLANE = 0.1f; //todo: specialization const
const float FAR_PLANE = 256.0f; //todo: specialization const 
layout (binding = 1) uniform sampler2D shadowMap;
float linearDepth(float depth)
{
	float z = depth * 2.0f - 1.0f; 
	return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));	
}
float textureProj(vec4 P, vec2 off)
{
	float shadow = 1.0;
	vec4 shadowCoord = P / P.w;
	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) 
	{
		float dist = texture( shadowMap, shadowCoord.st + off ).r;
		if ( shadowCoord.w > 0.0 && dist < shadowCoord.z) 
		{
			shadow = 0.15;
		}
	}
	return shadow;
}

float filterPCF(vec4 sc)
{
	ivec2 texDim = textureSize(shadowMap, 0);
	float scale = 1.2;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 2;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += textureProj(sc, vec2(dx*x, dy*y));
			count++;
		}
	
	}
	return shadowFactor / count;
}
void main() 
{
	float inshadow = filterPCF(inShadowCoord / inShadowCoord.w);
	outPosition = vec4(inPos, linearDepth(gl_FragCoord.z));
	outNormal = vec4(normalize(inNormal) * 0.5 + 0.5, 1.0);
	outAlbedo = vec4(1.0f, 0.765557f, 0.336057f, 1.0) ;
	outmix =vec4(1.0,0.1,0.0, 1.0);
}