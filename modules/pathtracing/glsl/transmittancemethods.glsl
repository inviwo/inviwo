#include "random.glsl"
#include "utils/shading.glsl"

/* --- 
    My stuff now
*/

float SimpleTracking(float T, float rayStep, float tau) {
    return T*exp(-tau*rayStep);
}

vec4 RMVolumeRender_simple(inout float T, float rayStep, float tau, vec4 tfSample, vec4 acc_radiance) {
    acc_radiance = acc_radiance + T*tau*tfSample*rayStep;
    

    return acc_radiance;
}

vec4 RMVolumeRender_simpleWithLight(float T, float rayStep, vec4 tfSample, vec4 acc_radiance, 
    vec3 samplePos, vec3 cameraDir, LightParameters light) {
    
    float lightCoeff = 1f/pow(distance(light.position, samplePos), 2f);
    
    vec4 color;
    

    // Made up colors of the particle
    vec3 sampleAmbient = tfSample.rgb;
    vec3 sampleDiffuse = tfSample.rgb;
    vec3 sampleSpecular = vec3(1f);

    vec3 toLight = normalize(light.position - samplePos);

    // only the volume is moved when the canvas is shifted. keep that in mind

    color.rgb = shadeSpecularPhongCalculation(light, tfSample.rgb, toLight, 
        toLight, cameraDir);

    color.w = 1f;
    acc_radiance += acc_radiance + T*tfSample.a*(color);

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
        //this is really slow since we are now doing 2 texture look ups multiple times, per ray.
        float sigma = applyTF(transferFunction, getNormalizedVoxel(volume, volumeParameters, r)).a*700;
        //float sigma = 1.5;
        if(random_1dto1d(hashSeed + 1) < sigma/sigma_upperbound) {
            break;
        }
    }

    return meanfreepath;
}

float WoodcockTracking_uvec2Hashseed(vec3 raystart, vec3 raydir, float raylength, uvec2 hashSeed, 
    sampler3D volume, VolumeParameters volumeParameters, sampler2D transferFunction, float sigma_upperbound ) {

    float meanfreepath = 0f; // tau

    vec3 r = vec3(0);
    while (meanfreepath < raylength) {
        meanfreepath += -log(random_2dto1d(hashSeed))/sigma_upperbound;

        r = raystart - meanfreepath*raydir;
        //this is really slow since we are now doing 2 texture look ups multiple times, per ray.
        float sigma = applyTF(transferFunction, getNormalizedVoxel(volume, volumeParameters, r)).a*700;
        //float sigma = 1.5;
        if(random_2dto1d(hashSeed + uvec2(1)) < sigma/sigma_upperbound) {
            break;
        }
    }

    return meanfreepath;
}

float RatioTrackingEstimator(inout vec3 raystart, vec3 raydir, float raylength, uint hashSeed, 
    sampler3D volume, VolumeParameters volumeParameters, sampler2D transferFunction, float sigma_upperbound) {
    
    float t = 0f;
    float T = 1f;

    do {
       float epsilon = random_1dto1d(hashSeed);

       t += -log(epsilon)/sigma_upperbound;
        if(t >= raylength) {
            break;
        }
        raystart = raystart + t*raydir;
        float sigma = applyTF(transferFunction, getNormalizedVoxel(volume, volumeParameters, raystart)).a*700;
        T = T * (1 - sigma/sigma_upperbound);

    } while(true);


    return T;
}