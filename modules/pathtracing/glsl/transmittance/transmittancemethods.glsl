#include "random.glsl"
#include "utils/shading.glsl"
#include "utils/intersection.glsl"
#include "utils/gradients.glsl"
#include "util/vectormatrixmethods.glsl"

float tfToExtinction(float s_max) { return s_max * 150f; }

// raystart, raydir and raylength ought to be in data coordinate
float woodcockTracking(vec3 raystart, vec3 raydir, float tStart, float tEnd, inout uint hashSeed,
                       sampler3D volume, VolumeParameters volumeParameters,
                       sampler2D transferFunction, float sigma_upperbound) {

    float invExtinction = 1.f / tfToExtinction(sigma_upperbound);
    float invSigmaUpperbound = 1.f / sigma_upperbound;
    float t = tStart;
    float sigmaSample;
    vec3 r = vec3(0);
    do {
        t += -log(random_1dto1d(pcgRehash(hashSeed))) * invExtinction;
        r = raystart + t * raydir;
        vec4 volumeSample = getNormalizedVoxel(volume, volumeParameters, r);
        sigmaSample = applyTF(transferFunction, volumeSample).a;

    } while (random_1dto1d(pcgRehash(hashSeed)) >= sigmaSample * invSigmaUpperbound && t <= tEnd);

    return t;
}

float ratioTrackingEstimator(inout vec3 raystart, vec3 raydir, float raylength, uint hashSeed,
                             sampler3D volume, VolumeParameters volumeParameters,
                             sampler2D transferFunction, float sigma_upperbound) {

    float t = 0f;
    float T = 1f;

    do {
        float epsilon = random_1dto1d(pcgRehash(hashSeed));

        t += -log(epsilon) / sigma_upperbound;
        if (t >= raylength) {
            break;
        }
        raystart = raystart + t * raydir;
        float sigma =
            applyTF(transferFunction, getNormalizedVoxel(volume, volumeParameters, raystart)).a *
            700;
        T = T * (1 - sigma / sigma_upperbound);

    } while (true);

    return T;
}
