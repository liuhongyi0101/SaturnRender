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
layout (binding = 2) uniform sampler2D albedoMap;
layout (binding = 3) uniform sampler2D normalMap;
layout (binding = 4) uniform sampler2D roughnessMap;
layout (binding = 5) uniform sampler2D metallicMap;
layout (binding = 6) uniform sampler2D aoMap;

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
vec3 perturbNormal()
{
	vec3 tangentNormal = texture(normalMap, inUV).xyz * 2.0 - 1.0;

	vec3 q1 = dFdx(inPos);
	vec3 q2 = dFdy(inPos);
	vec2 st1 = dFdx(inUV);
	vec2 st2 = dFdy(inUV);

	vec3 N = normalize(inNormal);
	vec3 T = normalize(q1 * st2.t - q2 * st1.t);
	vec3 B = -normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	return normalize(TBN * tangentNormal);
}
void main() 
{


    vec3 N = perturbNormal();
	vec3 albedo = pow(texture(albedoMap, inUV).rgb, vec3(2.2));
	float metallic = texture(metallicMap, inUV).r;
	float roughness = texture(roughnessMap, inUV).r;
	float ao = texture(aoMap, inUV).r;




	float inshadow = filterPCF(inShadowCoord / inShadowCoord.w);
	outPosition = vec4(inPos, linearDepth(gl_FragCoord.z));
	outNormal = vec4(N * 0.5 + 0.5, 1.0);
	outAlbedo = vec4(albedo, inshadow) ;
	outmix =vec4(metallic,roughness,ao, 1.0);
}