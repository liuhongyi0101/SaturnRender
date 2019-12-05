#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform UBO 
{
	
    mat4 uPerspective;
    vec3 uWorldExtent;
} ubo;

layout (binding = 1) uniform sampler2D samplerPositionDepth;
layout (binding = 2) uniform sampler2D samplerNormal;
layout (binding = 3) uniform sampler2D samplerColor;

layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outFragColor;
const float cb_maxDistance = 30000000.0;
const float cb_nearPlaneZ  = 0.0;
const float cb_farPlaneZ   = 50.0;
// if the first intersection found is at 1000000 z distance then the ray probably passed under that object
const float objectsThickness = 0.75;
const float cb_stride = 1.0;
const float cb_maxSteps = 20.0;
const float cb_zThickness = 0.00001;

vec4 viewToNDC(vec4 position) {
    vec4 hs = ubo.uPerspective * position;
    return hs / hs.w;
}

int traceScreenSpaceRay(vec3 orig,vec3 dir,out vec2 hitPixel,out vec3 hitPoint){

   
    int tr = 1, fa = 0;
    float step = 0.1;
    for(float i = 1.0; i <= 188.0; i++) {
        vec3 samplePoint = orig + dir * step * i;

        vec4 PS = viewToNDC(vec4(samplePoint, 1.0));
        
        float depthAtPS = abs(   texture(samplerPositionDepth, PS.xy * 0.5 + 0.5).w  );
        hitPixel.x = depthAtPS;
    }

    return fa;
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
    int intersection = traceScreenSpaceRay(rayOriginVS, rayDir, hitPixel, hitPoint);


	outFragColor = vec4(hitPixel, hitPoint.x, vDotN );
}

