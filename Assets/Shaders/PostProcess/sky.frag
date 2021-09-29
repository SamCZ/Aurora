#include "cb_sky.h"

in vec2 TexCoord;

uniform sampler2D DepthTexture;

out vec4 FragColor;

#define numInScatteringPoints 10
#define numOpticalDepthPoints 10

#define scatteringCoefficients atmosphereData.scatteringCoefficients

// Returs vector (dstToSphere, dstThroughSphere)
// If ray origin is inside sphere, dstToSphere = 0
// If ray misses sphere, dstToSphere = maxValue, dstToSphere = 0
vec2 raySphere(vec3 sphereCenter, float sphereRadius, vec3 rayOrigin, vec3 rayDir)
{
    vec3 offset = rayOrigin - sphereCenter;
    float a = 1.0f; // Set to dot(rayDir, rayDir) if rayDir might not be normalized
    float b = 2.0f * dot(offset, rayDir);
    float c = dot(offset, offset) - sphereRadius * sphereRadius;
    float d = b * b - 4 * a * c; // Discriminant for quadratic formula

    // Number of intersections: 0 when d < 0; 1 when d = 0; 2 when d > 0
    if(d > 0)
    {
        float s = sqrt(d);
        float dstToSphereNear = max(0, (-b - s) / (2 * a));
        float dstToSphereFar = (-b + s) / (2 * a);

        // Ignore intersenctions that occur behind the ray
        if(dstToSphereFar >= 0)
        {
            return vec2(dstToSphereNear, dstToSphereFar - dstToSphereNear);
        }
    }

    // Ray did not intersected
    return vec2(~0, 0);
}

vec3 planetCenter = vec3(0, 0, 0);
const vec3 dirToSun = normalize(vec3(0.5, 0.5, 0.5));
/*const float planetRadius = 1.0f;
const float atmosphereRadius = 2.0f;
const float densityFallOff = 4.0f;*/

float densityAtPoint(vec3 densitySamplePoint)
{
    float heightAboveSurface = length(densitySamplePoint - planetCenter) - planetRadius;
    float height01 = heightAboveSurface / (atmosphereRadius - planetRadius);
    float localDensity = exp(-height01 * densityFallOff) * (1 - height01);
    return localDensity;
}

float opticalDepth(vec3 rayOrigin, vec3 rayDir, float rayLength)
{
    vec3 densitySamplePoint = rayOrigin;
    float stepSize = rayLength / (numOpticalDepthPoints - 1);
    float opticalDepth = 0;

    for(uint i = 0; i < numOpticalDepthPoints; i++)
    {
        float localDensity = densityAtPoint(densitySamplePoint);
        opticalDepth += localDensity * stepSize;
        densitySamplePoint += rayDir * stepSize;
    }

    return opticalDepth;
}

vec3 calculateLight(vec3 rayOrigin, vec3 rayDir, float rayLength)
{
    vec3 inScatterPoint = rayOrigin;
    float stepSize = rayLength / (numInScatteringPoints - 1);
    vec3 inScatterLight = vec3(0);

    for(uint i = 0; i < numInScatteringPoints; i++)
    {
        float sunRayLength = raySphere(planetCenter, atmosphereRadius, inScatterPoint, dirToSun).y;
        float sunRayOpticalDepth = opticalDepth(inScatterPoint, dirToSun, sunRayLength);
        float viewRayOpticalDepth = opticalDepth(inScatterPoint, -rayDir, stepSize * i);
        vec3 transmittance = exp(-(sunRayOpticalDepth + viewRayOpticalDepth) * scatteringCoefficients.xyz);
        float localDensity = densityAtPoint(inScatterPoint);

        inScatterLight += localDensity * transmittance * scatteringCoefficients.xyz * stepSize;
        inScatterPoint += rayDir * stepSize;
    }

    return inScatterLight;
}

float linearize_depth(float d,float zNear,float zFar)
{
    return zNear * zFar / (zFar + d * (zNear - zFar));
}

void main()
{
    float x = 2.0 * gl_FragCoord.x / ViewPort.x - 1.0;
    float y = 2.0 * gl_FragCoord.y / ViewPort.y - 1.0;
    vec2 ray_nds = vec2(x, y);
    vec4 ray_clip = vec4(ray_nds, -1.0, 1.0);
    vec4 ray_view = InvProjection * ray_clip;
    ray_view = vec4(ray_view.xy, -1.0, 0.0);
    vec3 ray_world = (InvView * ray_view).xyz;
    ray_world = normalize(ray_world);

    vec3 color = vec3(0, 0, 0);

    vec3 rayOrigin = CameraPos.xyz;
    vec3 rayDir = ray_world;

    //planetCenter = rayOrigin;

    float depth = texelFetch(DepthTexture, ivec2(gl_FragCoord.xy), 0).r;
    float dstToSurface = linearize_depth(depth, 0.1f, 2000.0f);

    vec2 hitInfo = raySphere(planetCenter, atmosphereRadius, rayOrigin, rayDir);
    float dstToAtmosphere = hitInfo.x;
    float dstThroughAtmosphere = min(hitInfo.y, dstToSurface - dstToAtmosphere);

    if(dstThroughAtmosphere > 0)
    {
        const float epsilon = 0.0001;
        vec3 pointInAthmosphere = rayOrigin + rayDir * (dstToAtmosphere + epsilon);
        vec3 light = calculateLight(pointInAthmosphere, rayDir, dstThroughAtmosphere - epsilon * 2);

        color.rgb = light;
    }

    FragColor.rgb = color;
    FragColor.a = 1;
}