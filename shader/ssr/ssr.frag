#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform UBO 
{
	
    mat4 perspective;
    
} ubo;

layout (binding = 1) uniform sampler2D samplerPositionDepth;
layout (binding = 2) uniform sampler2D samplerNormal;
layout (binding = 3) uniform sampler2D samplerColor;
layout (binding = 4) uniform sampler2D frontZbuffer;

layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outFragColor;

const float NEAR_PLANE = 0.1f; //todo: specialization const
const float FAR_PLANE = 256.0f; //todo: specialization const 
// Consts should help improve performance
const float rayStep = 5.0;
const float minRayStep = 0.1;
const float maxSteps = 100;
const float searchDist = 10;
const float searchDistInv = 0.1;
const int numBinarySearchSteps = 50;
const float maxDDepth = 1.0;
const float maxDDepthInv = 1.0;
const float cb_zThickness = 0.3;
const float reflectionSpecularFalloffExponent = 3.0;
float g_depthbias = 0.001f;

vec4 viewToNDC(vec4 position) {
    vec4 hs =ubo.perspective * position;
    return hs / hs.w;
}

float linearDepth(float depth)
{
	float z = depth * 2.0f - 1.0f; 
	return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));	
}
float frontDepth(vec2 uv)
{
      
    float fdepth =texture(frontZbuffer,uv).r;
    fdepth = linearDepth(fdepth);
    return  -fdepth;
}

vec4 RayCast(vec3 dir, inout vec3 hitCoord, out float dDepth)
{
    dir *= rayStep;
    float depth;
    for(int i = 0; i < maxSteps; i++)
    {
        hitCoord += dir;
        vec4 projectedCoord = ubo.perspective * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

        depth = texture(samplerPositionDepth, projectedCoord.xy).z;
        float fdepth = frontDepth(projectedCoord.xy);
        float thickness = -depth +fdepth;
        dDepth = hitCoord.z - depth;
        
        if(hitCoord.z>depth && hitCoord.z <fdepth)
            return vec4(vec3(vec2( projectedCoord.xy), dDepth), 1.0);
    }
    return vec4(0.0, 0.0, 0.0, 0.0);
}

float PDnrand(in vec2 n) 
{
	return fract(sin(dot(n.xy, vec2(12.9898f, 78.233f)))* 43758.5453f);
}

float PDsrand(in vec2 n) 
{
	return PDnrand(n) * 2 - 1;
}


float maxRegenCount = 15.0;
float surfaceMargin = 0.19;
float rayTraceStride =3.7;
float rayTraceInitOffset =2.0;
float rayTraceMaxStep = 200.0;
float rayTraceHitThickness =0.05;






void main() 
{

    vec4 reflectColor = vec4(0.0, 0.0, 0.0, 0.0f);
    vec2 screenSize = vec2(1280.,720.);

	vec3 rayOrigin = texture(samplerPositionDepth, inUV).rgb;
	vec3 normal = normalize(texture(samplerNormal, inUV).rgb * 2.0 - 1.0);
    vec3 eyeDir   = normalize(rayOrigin);
    
    vec3 rayDir =  normalize(reflect(eyeDir,normal));

    vec2 origin = inUV * screenSize;
 
    vec3 rayendPoint = rayOrigin + rayDir * FAR_PLANE;

    vec4 sceneStart = ubo.perspective * vec4(rayOrigin, 1.0);

    vec4 sceneEnd =   ubo.perspective * vec4(rayendPoint, 1.0);

    bool permute = false;
    // 透视纠正

    float ks = 1.0 / sceneStart.w;
    float kd = 1.0 /sceneEnd.w;

    sceneStart *=ks;
    sceneEnd *=kd;

    rayOrigin *=ks;
    rayendPoint *=kd;

    sceneStart.xy = sceneStart.xy * 0.5 + 0.5;
    sceneEnd.xy = sceneEnd.xy * 0.5 + 0.5;
    
    sceneStart.xy *= screenSize;
    sceneEnd.xy *=screenSize;
    
    vec2 sceneRayDir = sceneEnd.xy - sceneStart.xy;
   
    float sqDistance = dot(sceneRayDir,sceneRayDir);

    sceneEnd.xy += step(sqDistance, 0.0001f) * vec2(0.01f);
    
    float divisions = length(sceneEnd.xy - sceneStart.xy);
	
	vec3 dV = (rayendPoint - rayOrigin) / divisions;
	float dK = (kd - ks) / divisions;
	vec2 traceDir = (sceneEnd.xy - sceneStart.xy) / divisions;
    float MAX_STEPS = 400.;
	float maxSteps = min(divisions, MAX_STEPS);
    float t =1.0;
    vec2 coord;
    vec2 texcoord;
	while (t < maxSteps)
	{
		coord = origin + traceDir * t;
		if (coord.x > screenSize.x || coord.y > screenSize.y || coord.x < 0 || coord.y < 0)
		{
			break;
		}

		float curDepth = (rayOrigin + dV * t).z;
		float k = ks + dK * t;
		curDepth /= k;
		texcoord = coord / screenSize;

		float storeFrontDepth = frontDepth(texcoord);
		float storeBackDepth = texture(samplerPositionDepth, texcoord).z;
		

		if ((curDepth < storeFrontDepth) && ((curDepth - storeFrontDepth) >= (storeBackDepth - storeFrontDepth)))
		{
			reflectColor =  texture(samplerColor, texcoord);	
			reflectColor.a = 0.4;
			break;	
		}
		t++;
	}




    // float stepDirection = sign(sceneRayDir.x);
    // float stepInterval = stepDirection / sceneRayDir.x;

    // vec3 dQ = (rayendPoint - rayOrigin) * stepInterval * rayTraceStride;
	// float dk = (kd - ks) * stepInterval * rayTraceStride;
	// vec2 dP = vec2(stepDirection, sceneRayDir.y * stepInterval) * rayTraceStride;

    
	// float jitter = PDsrand(vec2(inUV));
	// float init = rayTraceInitOffset ;

	// vec3 Q = rayOrigin + dQ * init;
	// float k = ks + dk * init;
	// vec2 P = sceneStart.xy + dP * init;

    // float end = sceneEnd.x * stepDirection;
	// float stepCount = 0.0f;

    // float prevZMax = rayOrigin.z;
	// float ZMin = prevZMax;
	// float ZMax = prevZMax;
	// float sampleZ = prevZMax - 100000;


    // vec2 hit;

   	// for (;((P.x * stepDirection) <= end) &&
	// 		(stepCount <= rayTraceMaxStep - 1) &&
	// 		((ZMax > sampleZ) || (ZMin < sampleZ - rayTraceHitThickness)) && 
	// 		sampleZ != 0.0f;
	// 		P += dP, Q.z += dQ.z, k += dk, stepCount++)
	// {
	// 	ZMin = prevZMax;
	// 	ZMax = (Q.z + dQ.z * 0.5f) / (k + dk * 0.5f);
	// 	prevZMax = ZMax;

	// 	if (ZMin < ZMax)
	// 	{
	// 		float t = ZMin;
	// 		ZMin = ZMax;
	// 		ZMax = t;
	// 	}

	// 	hit = permute ? P.yx : P;

		
    //     float depth = texture(samplerPositionDepth, hit).z;
	// 	sampleZ = depth;
	// }
   
    // vec3 hitNormal = normalize(texture(samplerNormal, hit).rgb * 2.0 - 1.0);
    // vec4 rayHitInfo;
    
	// rayHitInfo.rg = hit;
    // rayHitInfo.b = sampleZ;
    // rayHitInfo.a = float((ZMax < sampleZ) && (ZMin > sampleZ - rayTraceHitThickness) && (dot(hitNormal, rayDir) < 0));
    vec4 color =  texture(samplerColor, inUV);

         outFragColor =reflectColor;
  
    
}

