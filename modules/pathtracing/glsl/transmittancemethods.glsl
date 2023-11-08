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
        //this is really slow since we are now doing 2 texture look ups multiple times, per ray.
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
    vec3 sampleSpecular = vec3(1f);

    vec3 toLight = normalize(light.position - sampleWorldPos);

    // only the volume is moved when the canvas is shifted. keep that in mind

    color.rgb = shadeSpecularPhongCalculation(light, tfSample.rgb, toLight, 
        toLight, cameraDir);

    // Attenuate color from sampleWorldPos to lightPos
    // send a ray from sampleWorldPos to lightPos.
    // attenuate for the distance from samplePos to boundingbox intersection.
    // There are many examples of ray-box, -plane intersection tests, it is fairly trivial.
    // if our volume had a bounding box that would make this very simple, but Boron is sadly rectangular.

    // Function candidates:
    // bool rayPlaneIntersection (in PlaneParameters plane, in vec3 point, in vec3 rayDir, inout float t0, in float t1)
    // planes have no bounds, so we can get 3 intersections, the one with smallest t0 needs to be chosen.
    float t = 0f;
    float t_smallest = 100f;
    float t_test = 0f;
    //RayBBIntersection(bb, samplePos, toLight, t);

    // We get massive hickups at times and the rendering seems a lot more stuttery than before
    // this way of doing things must be VERY innefficient
    /**/
    if(rayPlaneIntersection(bb[0], samplePos, toLight, t_test, 100f)) {
        if(t_smallest > t_test) {
            t_smallest = t_test;
            t_test = 0f;
        }
    } 

    if(rayPlaneIntersection(bb[1], samplePos, toLight, t_test, 100f)) {
        if(t_smallest > t_test) {
            t_smallest = t_test;
            t_test = 0f;
        }
    } 

    if(rayPlaneIntersection(bb[2], samplePos, toLight, t_test, 100f)) {
        if(t_smallest > t_test) {
            t_smallest = t_test;
            t_test = 0f;
        }
    } 

    if(rayPlaneIntersection(bb[3], samplePos, toLight, t_test, 100f)) {
        if(t_smallest > t_test) {
            t_smallest = t_test;
            t_test = 0f;
        }
    } 

    if(rayPlaneIntersection(bb[4], samplePos, toLight, t_test, 100f)) {
        if(t_smallest > t_test) {
            t_smallest = t_test;
            t_test = 0f;
        }
    } 

    if(rayPlaneIntersection(bb[5], samplePos, toLight, t_test, 100f)) {
        if(t_smallest > t_test) {
            t_smallest = t_test;
            t_test = 0f;
        }
    } 

    if(t_smallest < 100f) {
        t = t_smallest;
    }
    
    float tau = 1f;    
    float meanfreepath_l = WoodcockTracking(samplePos, toLight, /*should be t*/ 0f, hashSeed, 
        volume, volParam, tf, 1f, tau);
    
    float Tl = 1f;
    Tl = SimpleTracking(Tl, meanfreepath_l, tfSample.a);

    color.w = 1f;
    acc_radiance += acc_radiance + T*tfSample.a*(color*Tl);

    return acc_radiance;
}