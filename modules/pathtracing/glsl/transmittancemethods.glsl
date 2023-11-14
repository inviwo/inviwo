#include "random.glsl"
#include "utils/shading.glsl"
#include "utils/intersection.glsl"
#include "renderingmethods.glsl"

/* --- 
    My stuff now
*/

// Returns how far to step, supposedly.
float WoodcockTracking(vec3 raystart, vec3 raydir, float raylength, float hashSeed, 
    sampler3D volume, VolumeParameters volumeParameters, sampler2D transferFunction, float sigma_upperbound, out float sigma) {

    float meanfreepath = 0f; // tau

    vec3 r = vec3(0);
    while (meanfreepath < raylength) {
        meanfreepath += -log(random_1dto1d(hashSeed))/sigma_upperbound;

        r = raystart - meanfreepath*raydir;
        sigma = applyTF(transferFunction, getNormalizedVoxel(volume, volumeParameters, r)).a;
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

float SimpleTracking(float T, float rayStep, float tau) {
    return T*exp(-tau*rayStep);
}

vec4 RMVolumeRender_simple(inout float T, float rayStep, float tau, vec4 tfSample, vec4 acc_radiance) {
    acc_radiance = acc_radiance + T*tau*tfSample*rayStep;
    

    return acc_radiance;
}

vec4 RMVolumeRender_SingleBounceLight(float T, float rayStep, sampler3D volume, VolumeParameters volParam, sampler2D tf, vec4 acc_radiance, 
    vec3 samplePos, vec3 cameraDir, LightParameters light, PlaneParameters[6] bb, float hashSeed) {
    
    //need worldpos
    vec3 sampleWorldPos = (volParam.textureToWorld*vec4(samplePos,1f)).xyz;
    
    vec4 color;
    
    vec4 voxel = getNormalizedVoxel(volume, volParam, samplePos);
    vec4 tfSample = applyTF(tf, voxel);

    // Made up colors of the particle
    vec3 sampleAmbient = tfSample.rgb;
    vec3 sampleDiffuse = tfSample.rgb;
    vec3 sampleSpecular = tfSample.rgb;

    vec3 toLight = normalize(light.position - sampleWorldPos);

    color.rgb = shadeSpecularPhongCalculation(light, tfSample.rgb, toLight, 
        toLight, cameraDir);

    // 
    float t = 0f;
    float tau = 1f;   
    float meanfreepath_l = 0f; 
    float Tl = 1f;


    /*
        Light Attenuation source to sample:
        Attenuate color from sampleWorldPos to lightPos
        send a ray from sampleWorldPos to lightPos.
        attenuate for the distance from samplePos to boundingbox intersection.
        There are many examples of ray-box, -plane intersection tests, it is fairly trivial.
        if our volume had a bounding box that would make this very simple, but Boron is sadly rectangular.

    */
    vec3 toLightTexture = (volParam.worldToTexture*vec4(toLight,1f)).xyz;

    //RayBBIntersection(bb, sampleWorldPos, toLight, t);
    //meanfreepath_l = WoodcockTracking(samplePos, toLightTexture, /*should be t*/ t, hashSeed, 
    //    volume, volParam, tf, 1f, tau);
    //Tl = SimpleTracking(Tl, meanfreepath_l, tau);

    // pseudo gamma
    float g = 1f;

    color.w = 1f;
    acc_radiance += acc_radiance + g*T*tau*(color*Tl);

    return acc_radiance;
}