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

layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outFragColor;

// Consts should help improve performance
const float rayStep = 0.25;
const float minRayStep = 0.1;
const float maxSteps = 40;
const float searchDist = 10;
const float searchDistInv = 0.1;
const int numBinarySearchSteps = 10;
const float maxDDepth = 1.0;
const float maxDDepthInv = 1.0;
const float cb_zThickness = 0.01;
const float reflectionSpecularFalloffExponent = 3.0;


vec4 viewToNDC(vec4 position) {
    vec4 hs =ubo.perspective * position;
    return hs / hs.w;
}

vec3 BinarySearch(vec3 dir, inout vec3 hitCoord, out float dDepth)
{
    float depth;


    for(int i = 0; i < numBinarySearchSteps; i++)
    {
        vec4 projectedCoord = ubo.perspective * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;


        depth = texture(samplerPositionDepth, projectedCoord.xy).z;


        dDepth = hitCoord.z - depth;


        if(dDepth > 0.0)
            hitCoord += dir;


        dir *= 0.5;
        hitCoord -= dir;    
    }


    vec4 projectedCoord = ubo.perspective * vec4(hitCoord, 1.0); 
    projectedCoord.xy /= projectedCoord.w;
    projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;


    return vec3(projectedCoord.xy, depth);
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


        dDepth = hitCoord.z - depth;


        if(dDepth < 0.0)
            return vec4(BinarySearch(dir, hitCoord, dDepth), 1.0);
    }


    return vec4(0.0, 0.0, 0.0, 0.0);
}


void main() 
{
	vec4 ssrColor = vec4(1.0,0.,0.,0.);
	vec3 rayOriginVS = texture(samplerPositionDepth, inUV).rgb;
	vec3 normal = normalize(texture(samplerNormal, inUV).rgb * 2.0 - 1.0);
    vec3 toPositionVS   = normalize(rayOriginVS);
    
    vec3 rayDir =  normalize(reflect(toPositionVS,normal));

    vec2 hitPixel = vec2(0.0f, 0.0f);
    vec3 hitPoint = vec3(0.0f, 0.0f, 0.0f);


    if ( normal.xyz == vec3(0.0)) {
        outFragColor = vec4(0, 0, 0, 1);
        return;
    }
    float vDotN = dot(toPositionVS, normal.xyz);
    
     // Ray cast
    float dDepth;

    vec3 viewPos =rayOriginVS;
    vec4 coords = RayCast(rayDir , rayOriginVS, dDepth);


    vec2 dCoords = abs(vec2(0.5, 0.5) - coords.xy);


    float screenEdgefactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);

    vec3 color =  texture(samplerColor, inUV).rgb;

    vec3 fcolor = texture(samplerColor, coords.xy).rgb;
        
     float mask = pow(0.2, reflectionSpecularFalloffExponent) *
        screenEdgefactor * clamp(-rayDir.z, 0.0, 1.0) *
        clamp((searchDist - length(viewPos - rayOriginVS)) * searchDistInv, 0.0, 1.0) * coords.w;
    // Get color
    color = mix(color,fcolor,coords.w);
    outFragColor = vec4(color,1.0);
}

