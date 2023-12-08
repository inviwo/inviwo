#include "random.glsl"
#include "utils/shading.glsl"
#include "utils/intersection.glsl"
#include "utils/gradients.glsl"
#include "util/vectormatrixmethods.glsl"

// pseudo enums
const uint WOODCOCK = 101;
const uint RATIO = 102;
const uint RESIDUALRATIO = 103;

float tfToExtinction(float s_max) { return s_max * 150f; }

// raystart, raydir and raylength ought to be in data coordinate
float woodcockTracking(vec3 raystart, vec3 raydir, float tStart, float tEnd, inout uint hashSeed,
                       sampler3D volume, VolumeParameters volumeParameters,
                       sampler2D transferFunction, float sigma_upperbound) {

    float invMaxExtinction = 1.f / tfToExtinction(sigma_upperbound);
    float invSigmaUpperbound = 1.f / sigma_upperbound;
    float t = tStart;
    float sigmaSample;
    vec3 r = vec3(0);
    do {
        t += -log(random_1dto1d(pcgRehash(hashSeed))) * invMaxExtinction;
        r = raystart + t * raydir;
        vec4 volumeSample = getNormalizedVoxel(volume, volumeParameters, r);
        sigmaSample = applyTF(transferFunction, volumeSample).a;

    } while (random_1dto1d(pcgRehash(hashSeed)) >= sigmaSample * invSigmaUpperbound && t <= tEnd);

    return t;
}

float ratioTrackingEstimator(vec3 raystart, vec3 raydir, float tStart, float tEnd,
                             inout uint hashSeed, sampler3D volume,
                             VolumeParameters volumeParameters, sampler2D transferFunction,
                             float sigma_upperbound) {

    float invMaxExtinction = 1.f / tfToExtinction(sigma_upperbound);
    float invSigmaUpperbound = 1.f / sigma_upperbound;
    float t = tStart;
    float sigmaSample;
    vec3 r = vec3(0);
    float T = 1.f;  // transmittance
    do {
        t += -log(random_1dto1d(pcgRehash(hashSeed))) * invMaxExtinction;
        if (t >= tEnd) {
            break;
        }
        r = r + t * raydir;
        vec4 volumeSample = getNormalizedVoxel(volume, volumeParameters, r);
        sigmaSample = applyTF(transferFunction, volumeSample).a;
        T *= (1 - sigmaSample * invSigmaUpperbound);

    } while (true);

    return T;
}

float residualRatioTracking(vec3 raystart, vec3 raydir, float tStart, float tEnd,
                            inout uint hashSeed, sampler3D volume,
                            VolumeParameters volumeParameters, sampler2D transferFunction,
                            float sigma_upperbound, float sigma_control) {

    if (sigma_upperbound < 2e-6) {
        return 1.f;
    }
    float invMaxExtinction = 1.f / tfToExtinction(sigma_upperbound);
    float invSigmaUpperbound = 1.f / sigma_upperbound;
    float t = tStart;
    float sigmaSample;
    vec3 r = vec3(0);
    float Tc = exp(-tfToExtinction(sigma_control) * (tEnd - tStart));
    float Tr = 1.f;  // transmittance

    do {
        t += -log(random_1dto1d(pcgRehash(hashSeed))) * invMaxExtinction;
        if (t >= tEnd) {
            break;
        }
        r = r + t * raydir;
        vec4 volumeSample = getNormalizedVoxel(volume, volumeParameters, r);
        sigmaSample = applyTF(transferFunction, volumeSample).a;
        Tr *= (1 - (sigmaSample - sigma_control) * invSigmaUpperbound);

    } while (true);

    return Tr * Tc;
}

float transmittance(uint METHOD, vec3 raystart, vec3 raydir, float tStart, float tEnd,
                    inout uint hashSeed, sampler3D volume, VolumeParameters volumeParameters,
                    sampler2D transferFunction) {

    float sigma_upperbound = 1.0f;
    float sigma_control = 0.5f;

    switch (METHOD) {
        case WOODCOCK:
            float meanFreePath =
                woodcockTracking(raystart, raydir, tStart, tEnd, hashSeed, volume, volumeParameters,
                                 transferFunction, sigma_upperbound);
            return meanFreePath >= tEnd ? 1f : 0f;
        case RATIO:
            return ratioTrackingEstimator(raystart, raydir, tStart, tEnd, hashSeed, volume,
                                          volumeParameters, transferFunction, sigma_upperbound);
        case RESIDUALRATIO:
            return residualRatioTracking(raystart, raydir, tStart, tEnd, hashSeed, volume,
                                         volumeParameters, transferFunction, sigma_upperbound,
                                         sigma_control);
    }

    return 0f;
}