#include "random.glsl"
#include "utils/shading.glsl"
#include "utils/intersection.glsl"
#include "renderingmethods.glsl"

/* --- 
    My stuff now
*/

float tfToExtinction(float s_max, float toExtMod) {
    return s_max*toExtMod;
}

// Returns how far to step, supposedly.
// raystart, raydir and raylength ought to be in data coordinate
float WoodcockTracking(vec3 raystart, vec3 raydir, float tStart, float tEnd, inout uint hashSeed, 
    sampler3D volume, VolumeParameters volumeParameters, sampler2D transferFunction, float sigma_upperbound, out float sigma) {
    
    float invExtinction = 1.f/tfToExtinction(sigma_upperbound, 1.0f);
    float invSigmaUpperbound = 1.f/sigma_upperbound;
    float t = tStart; 
    float sigmaSample;
    vec3 r = vec3(0);
    do {    
        hashSeed = pcg(hashSeed);
        t += -log(random_1dto1d(hashSeed))*invExtinction;
        hashSeed = pcg(hashSeed);
        r = raystart + t*raydir;
        vec4 volumeSample = getNormalizedVoxel(volume, volumeParameters, r);
        sigmaSample = applyTF(transferFunction, volumeSample).a;

    } while (random_1dto1d(hashSeed) >= sigmaSample*invSigmaUpperbound && t <= tEnd);

    return t;
}

float WoodcockTracking_InvT(vec3 raystart, vec3 raydir, float tStart, float tEnd, inout uint hashSeed, 
    sampler3D volume, VolumeParameters volumeParameters, sampler2D transferFunction, float sigma_upperbound, out float sigma) {
    
    float invExtinction = 1.f/tfToExtinction(sigma_upperbound, 150f);
    float invSigmaUpperbound = 1.f/sigma_upperbound;
    float t = tStart; 
    float sigmaSample;
    vec3 r = vec3(0);
    while (t < tEnd) {
        t = t - log(1f - random_1dto1d(hashSeed))*invSigmaUpperbound;   

        r = raystart - t*raydir;
        sigma = applyTF(transferFunction, getNormalizedVoxel(volume, volumeParameters, r)).a;
        if(random_1dto1d(hashSeed + 1) < sigma*sigma_upperbound) {
            break;
        }
    }
    return t;
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
    vec3 samplePos, vec3 cameraDir, LightParameters light, PlaneParameters[6] bb, uint hashSeed) {
    
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

    /*
        Light Attenuation source to sample:
        Attenuate color from sampleWorldPos to lightPos
        send a ray from sampleWorldPos to lightPos.
        attenuate for the distance from samplePos to boundingbox intersection.
        There are many examples of ray-box, -plane intersection tests, it is fairly trivial.
        if our volume had a bounding box that would make this very simple, but Boron is sadly rectangular.

        look for estimateDirectFromOneLightSource
        estimateLight {
            estimateDirectFromOneLightSource {
                sampleLightSource
                rayBoxIntersection
                transmittanceTracking
                applyShading {
                    if(shading_type == BLINN_PHONG) {
                        return BLINN_PHONG_SHADING()
                    }
                }
            }
        }

    */

    
    //color.rgb = shadeSpecularPhongCalculation(light, tfSample.rgb, /*what is the normal of a particle*/ -cameraDir, 
    //    toLight, cameraDir);
    
    color.rgb = shadeBlinnPhong(light, tfSample.rgb, tfSample.rgb, tfSample.rgb,
        /*what is the normal of a particle*/ sampleWorldPos, -cameraDir, cameraDir);
    
    float t0 = 0.0f;
    float t1;
    float tau = 1f;   
    float meanfreepath_l = 0f; 
    float Tl = 1f;


    
    vec3 toLightTexture = (volParam.worldToTexture*vec4(toLight,1f)).xyz;

    RayBBIntersection_TextureSpace(samplePos, toLightTexture, t0, t1);
    meanfreepath_l = WoodcockTracking(samplePos, toLightTexture, /*should be t*/ 0f, t1, hashSeed, 
        volume, volParam, tf, 1f, tau);
    Tl = meanfreepath_l >= t1 ? 1.0f : 0.0f;

    // pseudo gamma
    float g = 1.0f;

    color.a = tfSample.a;
    
    if(t1 > meanfreepath_l) {
        
    }
    else {  
        
    }
    acc_radiance = g*color*Tl;

    return acc_radiance;
}