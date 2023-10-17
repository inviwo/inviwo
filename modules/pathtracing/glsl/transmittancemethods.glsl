#include "random.glsl"

/* --- 
    My stuff now
*/

vec4 HelloWorld(vec4 inVec4) {


    vec4 outVec4 = vec4(1 - inVec4.x, 1 - inVec4.y, 1- inVec4.z, inVec4.w);
    
    float xDiff = abs(outVec4.x - inVec4.x);
    float yDiff = abs(outVec4.y - inVec4.y);
    float zDiff = abs(outVec4.z - inVec4.z);

    outVec4.x = outVec4.x/xDiff;
    outVec4.y = outVec4.y/yDiff;
    outVec4.z = outVec4.z/zDiff;
    return inVec4;
}

vec4 RMVolumeRender_simple(inout float T, float rayStep, float tau, vec4 tfSample, vec4 acc_radiance) {
    acc_radiance = acc_radiance + T*tau*tfSample*rayStep;
    T = T*exp(-tau*rayStep);

    return acc_radiance;
}

vec4 RMVolumeRender_simpleWithLight(inout float T, float rayStep, float tau, vec4 tfSample, vec4 acc_radiance, 
    vec3 samplePos, vec3 lightPos, float lightRadiance) {
    
    float lightCoeff = lightRadiance/pow(distance(lightPos, samplePos), 2f);
    vec4 lightColor = vec4(1.0f, 0.8f, 0.8f, 1.0f)*lightCoeff;

    lightColor.w = 1f;

    acc_radiance = acc_radiance + T*tau*(tfSample + lightColor)*rayStep;
    T = T*exp(-tau*rayStep);

    return acc_radiance;
}

// Returns how far to step, supposedly.
float WoodcockTracking(vec3 raystart, vec3 raydir, float raylength, float hashSeed, 
    sampler3D volume, VolumeParameters volumeParameters, sampler2D transferFunction, float sigma_upperbound ) {

    float meanfreepath = 0f; // tau

    vec3 r = vec3(0);
    while (meanfreepath < raylength) {
        meanfreepath += -log(random_1dto1d(hashSeed))/sigma_upperbound;

        r = raystart - meanfreepath*raydir;
        vec4 voxel = getNormalizedVoxel(volume, volumeParameters, r);
        float sigma = applyTF(transferFunction, voxel).a*700;

        if(random_1dto1d(hashSeed) < sigma/sigma_upperbound) {
            break;
        }
    }

    return meanfreepath;
}
