#include "random.glsl"
#include "utils/shading.glsl"
#include "utils/intersection.glsl"
#include "utils/gradients.glsl"

// pseudo enums
const uint WOODCOCK = 101;
const uint RATIO = 102;
const uint RESIDUALRATIO = 103;

const float REFSAMPLINGINTERVAL = 150.0;

float opacityToExtinction(float s_max) { return s_max * REFSAMPLINGINTERVAL; }

float woodcockTracking(vec3 raystart, vec3 raydir, float tStart, float tEnd, inout uint hashSeed,
                       sampler3D volume, VolumeParameters volumeParameters,
                       sampler2D transferFunction, float opacityUpperbound) {

    float invMaxExtinction = 1.f / opacityToExtinction(opacityUpperbound);
    float invOpacitUpperbound = 1.f / opacityUpperbound;
    float t = tStart;
    float opacity;
    vec3 samplePos = vec3(0);
    do {
        t += -log(randomize(hashSeed)) * invMaxExtinction;
        samplePos = raystart + t * raydir;
        vec4 volumeSample = getNormalizedVoxel(volume, volumeParameters, samplePos);
        opacity = applyTF(transferFunction, volumeSample).a;

    } while (randomize(hashSeed) >= opacity * invOpacitUpperbound && t <= tEnd);

    return t;
}

float ratioTrackingTransmittance(vec3 raystart, vec3 raydir, float tStart, float tEnd,
                                 inout uint hashSeed, sampler3D volume,
                                 VolumeParameters volumeParameters, sampler2D transferFunction,
                                 float opacityUpperbound) {

    float invMaxExtinction = 1.f / opacityToExtinction(opacityUpperbound);
    float invOpacitUpperbound = 1.f / opacityUpperbound;
    float t = tStart;
    float opacity;
    vec3 samplePos = vec3(0);
    float T = 1.f;  // transmittance
    do {
        t += -log(randomize(hashSeed)) * invMaxExtinction;
        if (t >= tEnd) {
            break;
        }
        samplePos = samplePos + t * raydir;
        vec4 volumeSample = getNormalizedVoxel(volume, volumeParameters, samplePos);
        opacity = applyTF(transferFunction, volumeSample).a;
        T *= (1 - opacity * invOpacitUpperbound);

    } while (true);

    return T;
}

float residualRatioTrackingTransmittance(vec3 raystart, vec3 raydir, float tStart, float tEnd,
                                         inout uint hashSeed, sampler3D volume,
                                         VolumeParameters volumeParameters,
                                         sampler2D transferFunction, float opacityUpperbound,
                                         float opacityControl) {

    if (opacityUpperbound < 2e-6) {
        return 1.f;
    }
    float invMaxExtinction = 1.f / opacityToExtinction(opacityUpperbound);
    float invOpacitUpperbound = 1.f / opacityUpperbound;
    float t = tStart;
    float opacity;
    vec3 samplePos = vec3(0);
    float Tc = exp(-opacityToExtinction(opacityControl) * (tEnd - tStart));
    float Tr = 1.f;  // transmittance

    do {
        t += -log(randomize(hashSeed)) * invMaxExtinction;
        if (t >= tEnd) {
            break;
        }
        samplePos = samplePos + t * raydir;
        vec4 volumeSample = getNormalizedVoxel(volume, volumeParameters, samplePos);
        opacity = applyTF(transferFunction, volumeSample).a;
        Tr *= (1 - (opacity - opacityControl) * invOpacitUpperbound);

    } while (true);

    return Tr * Tc;
}

float transmittance(uint METHOD, vec3 raystart, vec3 raydir, float tStart, float tEnd,
                    inout uint hashSeed, sampler3D volume, VolumeParameters volumeParameters,
                    sampler2D transferFunction) {

    float opacityUpperbound = 1.0f;
    float opacityControl = 0.5f;

    switch (METHOD) {
        case WOODCOCK:
            float meanFreePath =
                woodcockTracking(raystart, raydir, tStart, tEnd, hashSeed, volume, volumeParameters,
                                 transferFunction, opacityUpperbound);
            return meanFreePath >= tEnd ? 1f : 0f;
        case RATIO:
            return ratioTrackingTransmittance(raystart, raydir, tStart, tEnd, hashSeed, volume,
                                              volumeParameters, transferFunction,
                                              opacityUpperbound);
        case RESIDUALRATIO:
            return residualRatioTrackingTransmittance(raystart, raydir, tStart, tEnd, hashSeed,
                                                      volume, volumeParameters, transferFunction,
                                                      opacityUpperbound, opacityControl);
    }

    return 0f;
}