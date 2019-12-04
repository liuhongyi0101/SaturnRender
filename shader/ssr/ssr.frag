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

    // float rayLength = dir.z < 0.0 ? cb_farPlaneZ * abs(dir.z) + orig.z : 
    //                                 abs(orig.z) * abs(dir.z); 
    
    // vec3 endPoint  = orig + dir * rayLength;
    // endPoint = scaleLengthAgainstViewFrustum(orig, endPoint);

    // RAYLENGTH E' ZERO SE LA DIREZIONE RIFLESSA E' VERSO +Z
    // hitPoint = vec3(rayLength / 50.0);
    int tr = 1, fa = 0;
    float step = 0.1;
    for(float i = 1.0; i <= 188.0; i++) {
        vec3 samplePoint = orig + dir * step * i;

        vec4 PS = viewToNDC(vec4(samplePoint, 1.0));
        // abs because every depth is negative
        float depthAtPS = abs(   texture(samplerPositionDepth, PS.xy * 0.5 + 0.5).w  );
        if (depthAtPS < 0.001) depthAtPS = ubo.uWorldExtent.z;

        // if (i > 79.0) { 
        //     hitPoint.x = length(samplePoint - orig);
        //     hitPixel.xy = PS.xy * 0.5 + 0.5; // vec2(hitPoint.x);
        //     return true;
        // }

        // if we have an intersection...
        // abs because every depth is negative
        if (depthAtPS < abs(samplePoint.z * 0.98)) {


            // // attempt a linear search
            // // attempt a linear search
            // // attempt a linear search
            // vec3 lo = orig + dir * step * (i-1.0);
            // vec3 hi = samplePoint;

            // for (float j = 0.0; j < 8.0; j++) {
            //     vec3 newEstimate = (lo + hi) / 2.0;
            //     vec4 nePS = viewToNDC(vec4(newEstimate, 1.0));
            //     float nedepthAtPS = abs(   texture(uPositionBuffer, nePS.xy * 0.5 + 0.5).z  );
                
            //     if( nedepthAtPS < abs(newEstimate.z) ) {
            //         hi = newEstimate;
            //     } else {
            //         lo = newEstimate;
            //     }
            // }

            // samplePoint = hi;
            // PS = viewToNDC(vec4(samplePoint, 1.0));
            // depthAtPS = abs(   texture(uPositionBuffer, PS.xy * 0.5 + 0.5).z  );
            // // attempt a linear search - END 
            // // attempt a linear search - END 
            // // attempt a linear search - END 

            // Trying to solve the z-cutoff problem ...            
            if (abs(orig.z) > depthAtPS) return fa;
            
            // think about the following line.. if the difference in depth between the sample point and the hit point is
            // bigger than a certain (small) value we're not hitting the visible part of the face,
            // but something that's either a backface or anyway not visible on screen
            // since we're interested in showing the reflections only of the visible part of an object, we're
            // skipping the points behind/not visible of the object we're hitting
            // this way we solve the z-cutoff problem

            // see figure 1
            if (abs(samplePoint.z) - depthAtPS > objectsThickness) return fa;
            // in this case I'm just skipping iteration since I want the i to go to the max loop iteration
            // so that we can instead sample the cubemap
            // if (abs(samplePoint.z) - depthAtPS > objectsThickness) continue;
            // Trying to solve the z-cutoff problem ... - END            



            if (i > 79.0) { return fa; }
            if (depthAtPS > ubo.uWorldExtent.z) return fa;
            if (abs(samplePoint.z) > ubo.uWorldExtent.z) return fa;
            if (abs(samplePoint.x) > ubo.uWorldExtent.x) return fa;
            if (abs(samplePoint.y) > ubo.uWorldExtent.y) return fa;
            if (abs(PS.z) >= 1.0) return fa;
            if (abs(PS.x) >= 1.0) return fa;
            if (abs(PS.y) >= 1.0) return fa;

            hitPoint.x = length(samplePoint - orig);
            


            hitPixel.xy = PS.xy * 0.5 + 0.5; // vec2(hitPoint.x);
            return tr;
        }
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


	outFragColor = vec4(hitPixel, hitPoint.x, vDotN) * intersection + (vec4(0.0) * 1-intersection);
}

