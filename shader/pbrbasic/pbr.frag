#version 450
#extension GL_GOOGLE_include_directive : require


layout (location = 0) in vec3 inWorldPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inShadowCoord;

layout (set = 0,binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 model;
	mat4 view;
	mat4 lightSpace;
	vec3 camPos;
} ubo;

layout (binding = 1) uniform UBOShared {
	vec4 lights[4];
} uboParams;

layout (binding = 2) uniform samplerCube samplerIrradiance;
layout (binding = 3) uniform samplerCube prefilteredMap;
layout (binding = 4) uniform sampler2D samplerBRDFLUT;

layout (binding = 5) uniform sampler2D albedoMap;
layout (binding = 6) uniform sampler2D normalMap;
layout (binding = 7) uniform sampler2D aoMap;
layout (binding = 8) uniform sampler2D metallicMap;
layout (binding = 9) uniform sampler2D roughnessMap;
layout (binding = 10) uniform sampler2D shadowMap;
#include "ibl.glsl"
#include "shadow.glsl"
layout (location = 0) out vec4 outColor;

layout(push_constant) uniform PushConsts {
	layout(offset = 12) float roughness;
	layout(offset = 16) float metallic;
	layout(offset = 20) float r;
	layout(offset = 24) float g;
	layout(offset = 28) float b;
} material;

//#define ROUGHNESS_PATTERN 1

vec3 materialcolor()
{
	return pow(texture(albedoMap, inUV).rgb, vec3(2.2));
}

// Fresnel function ----------------------------------------------------
vec3 F_Schlick(float cosTheta, float metallic)
{
	vec3 F0 = mix(vec3(0.04), materialcolor(), metallic); // * material.specular
	vec3 F = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0); 
	return F;    
}

// Specular BRDF composition --------------------------------------------

vec3 BRDF(vec3 L, vec3 V, vec3 N, float metallic, float roughness)
{
	// Precalculate vectors and dot products	
	vec3 H = normalize (V + L);
	float dotNV = clamp(dot(N, V), 0.0, 1.0);
	float dotNL = clamp(dot(N, L), 0.0, 1.0);
	float dotLH = clamp(dot(L, H), 0.0, 1.0);
	float dotNH = clamp(dot(N, H), 0.0, 1.0);

	// Light color fixed
	vec3 lightColor = vec3(1.0);

	vec3 color = vec3(0.0);

	if (dotNL > 0.0)
	{
		float rroughness = max(0.05, roughness);
		// D = Normal distribution (Distribution of the microfacets)
		float D = D_GGX(dotNH, roughness); 
		// G = Geometric shadowing term (Microfacets shadowing)
		float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
		// F = Fresnel factor (Reflectance depending on angle of incidence)
		vec3 F = F_Schlick(dotNV, metallic);

		vec3 spec = D * F * G / (4.0 * dotNL * dotNV);

		color += spec * dotNL * lightColor;
	}

	return color;
}

// ----------------------------------------------------------------------------
void main()
{		  
	vec3 N = perturbNormal();
	vec3 V = normalize(ubo.camPos - inWorldPos);
	vec3 R = reflect(-V, N); 
	
	float metallic = texture(metallicMap, inUV).r;
	float roughness = texture(roughnessMap, inUV).r;
	// Add striped pattern to roughness based on vertex position
#ifdef ROUGHNESS_PATTERN
	roughness = max(roughness, step(fract(inWorldPos.y * 2.02), 0.5));
#endif

	// Specular contribution
vec3 F0 = mix(vec3(0.04), materialcolor(), metallic);
	vec3 Lo = vec3(0.0);
	vec3 diffusecolor = vec3(0.0);
	for (int i = 0; i < uboParams.lights.length(); i++) {
		vec3 L = normalize(uboParams.lights[i].xyz - inWorldPos);
		Lo += BRDF(L, V, N, metallic, roughness);
		float dotNL = clamp(dot(N, L), 0.0, 1.0);
	    diffusecolor += vec3(1.0)*dotNL;
	};

	vec2 brdf = texture(samplerBRDFLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
	vec3 reflection = prefilteredReflection(R, roughness).rgb;	
	vec3 irradiance = texture(samplerIrradiance, N).rgb;
	// Combine with ambient

	vec3 F = F_SchlickR(max(dot(N, V), 0.0), F0, roughness);

	// Specular reflectance
	vec3 specular = reflection * (F * brdf.x + brdf.y);
	
	
	diffusecolor *= materialcolor() *(1.0- metallic);
	diffusecolor =irradiance *  diffusecolor;

	vec3 color  = diffusecolor + Lo+specular;
	 color *= texture(aoMap, inUV).rrr;

	float inshadow = filterPCF(inShadowCoord / inShadowCoord.w);
	// Tone mapping
	color = Uncharted2Tonemap(color *3.0);
	color = color * (1.0f / Uncharted2Tonemap(vec3(11.2f)));	

	// Gamma correct
	color = pow(color, vec3(0.4545));

	outColor = vec4(color, 1.0);
}