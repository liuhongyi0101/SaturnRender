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

// vec4 RayCast(vec3 dir, inout vec3 hitCoord, out float dDepth)
// {
//     dir *= rayStep;
//     float depth;
//     for(int i = 0; i < maxSteps; i++)
//     {
//         hitCoord += dir;
//         vec4 projectedCoord = ubo.perspective * vec4(hitCoord, 1.0);
//         projectedCoord.xy /= projectedCoord.w;
//         projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

//         depth = texture(samplerPositionDepth, projectedCoord.xy).z;
//         float fdepth = frontDepth(projectedCoord.xy);
//         float thickness = -depth +fdepth;
//         dDepth = hitCoord.z - depth;
        
//         if(hitCoord.z>depth && hitCoord.z <fdepth)
//             return vec4(vec3(vec2( projectedCoord.xy), dDepth), 1.0);
//     }
//     return vec4(0.0, 0.0, 0.0, 0.0);
// }

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
float rayTraceStride =1.0;
float rayTraceInitOffset =2.0;
float rayTraceMaxStep = 200.0;
float rayTraceHitThickness =0.05;
vec2 coord;
float ReconstructLinearDepth(vec2 uv)
{
    float fdepth =texture(frontZbuffer,uv).r;
	return -1.0f * 0.1  / fdepth;
}
float ReconstructLinearbDepth(vec2 uv)
{
    float fdepth =texture(samplerPositionDepth,uv).w;
	return -1.0f * 0.1  / fdepth;
}



void main() 
{

    vec4 reflectColor = vec4(0.0, 0.0, 0.0, 0.0);
    vec2 screenSize = vec2(1280.,720.);
    vec2 texcoord = inUV;
	vec3 v0 = texture(samplerPositionDepth, inUV).rgb;
	vec3 normal = normalize(texture(samplerNormal, inUV).rgb * 2.0 - 1.0);
    vec3 eyeDir   = normalize(v0);
    float csNearPlane = 0.1;
    vec3 rayDir =  normalize(reflect(eyeDir,normal));

    vec2 origin = inUV * screenSize;
    float maxDistance = 200.0f;
   

    float rayLength = (v0.z + rayDir.z * maxDistance > csNearPlane) ? (csNearPlane - v0.z) / rayDir.z : maxDistance;

     vec3 v1 = v0 + rayDir * (rayLength);
    vec4 p0 = ubo.perspective * vec4(v0, 1.0);

    vec4 p1 =   ubo.perspective * vec4(v1, 1.0);

    bool permute = false;
    // 透视纠正

    float k0 = 1.0 / p0.w;
    float k1 = 1.0 /p1.w;

    p0 *=k0;
    p1 *=k1;

    v0 *=k0;
    v1 *=k1;

    p0.xy = p0.xy * 0.5 + 0.5;
    p1.xy = p1.xy * 0.5 + 0.5;
    
    p0.xy *= screenSize;
    p1.xy *=screenSize;
    
    vec2 sceneRayDir = p1.xy - p0.xy;
   
    float sqDistance = dot(sceneRayDir,sceneRayDir);

    p1.xy += step(sqDistance, 0.0001f) * vec2(0.01f);
    
    float divisions = length(p1.xy - p0.xy);
	

    // float stepDirection = sign(sceneRayDir.x);
    // float stepInterval = stepDirection / sceneRayDir.x;

    // vec3 dQ = (v1 - v0) * stepInterval * rayTraceStride;
	// float dk = (kd - ks) * stepInterval * rayTraceStride;
	// vec2 dP = vec2(stepDirection, sceneRayDir.y * stepInterval) * rayTraceStride;

    
	// float jitter = PDsrand(vec2(inUV));
	// float init = rayTraceInitOffset ;

	// vec3 Q = v0 + dQ * init;
	// float k = ks + dk * init;
	// vec2 P = p0.xy + dP * init;

    // float end = p1.x * stepDirection;
	// float stepCount = 0.0f;
   
  
    vec2 hit;
    float mask = 0.0;

	vec3 dV = (v1 - v0) / divisions;
	float dK = (k1 - k0) / divisions;
	vec2 traceDir = (p1.xy - p0.xy) / divisions;
    float MAX_STEPS = 500.;
	float maxSteps = min(divisions, MAX_STEPS);
    float t =1.0;
	while (t < 500.)
	{
		coord = origin + traceDir * t * 5.0;
		if (coord.x > screenSize.x || coord.y > screenSize.y || coord.x < 0 || coord.y < 0)
		{
			break;
		}
 
		float curDepth = (v0 + dV * t).z;
		float k = k0 + dK * t;
		curDepth /= k;
		texcoord = (coord) / screenSize;
		float storeFrontDepth = texture(samplerPositionDepth, texcoord).z;
		
        float storeBackDepth = ReconstructLinearDepth(texcoord);
		
		if ((curDepth > storeFrontDepth) )
		{
			reflectColor = texture(samplerColor, texcoord);	
			reflectColor.a = 0.4;
			break;	
		}
		t++;
	}
 

   
        vec4 color =  texture(samplerColor, inUV);
        outFragColor =reflectColor;
    
         
  
    
}

