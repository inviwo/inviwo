#include "random.glsl"
#include "utils/shading.glsl"
#include "utils/intersection.glsl"
#include "utils/gradients.glsl"
#include "util/rayminmax.glsl"

// Method Name pseudo enum
const int WOODCOCK = 0;
const int RATIO = 1;
const int RESIDUALRATIO = 2;

#define REFSAMPLINGINTERVAL 150.0

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

float partitionedTransmittanceTracking(int METHOD, vec3 raystart, vec3 raydir, float tStart,
                                       float tEnd, inout uint hashSeed, sampler3D volume,
                                       VolumeParameters volumeParameters,
                                       sampler2D transferFunction, sampler3D opacity,
                                       VolumeParameters opacityParameters) {

    mat4 volumeTexToIndMat = volumeParameters.textureToIndex;

    vec3 x1 = (volumeTexToIndMat * vec4(raystart + tStart * raydir, 1.0f)).xyz + 0.5f;
    vec3 x2 = (volumeTexToIndMat * vec4(raystart + tEnd * raydir, 1.0f)).xyz + 0.5f;

    vec3 cellDim = volumeParameters.dimensions * opacityParameters.reciprocalDimensions;
    ivec3 cellCoord = ivec3(x1 / cellDim);

    if (tEnd <= tStart) {
        return 1.f;
    }

    ivec3 cellCoordEnd, di;
    vec3 dt, deltatx;
    setupUniformGridTraversal(x1, x2, cellDim, ivec3(volumeParameters.dimensions), cellCoord, cellCoordEnd,
                              di, dt, deltatx);
    float dirLen = tEnd - tStart;
    float t = 0.f;

    bool continueTraversal = true;
    float T = 1.f;
    while(continueTraversal) {
        vec3 minMax = getNormalizedVoxel(opacity, opacityParameters, cellCoord).xyz;
        float tPrev = t;
        continueTraversal = stepToNextCellHit(deltatx, di, cellCoordEnd, dt, cellCoord, t);
        if(minMax.y <= 0) {
            continue;
        }
        
        float t0 = tStart + dirLen * tPrev;
        float t1 = tStart + dirLen * min(1.f, t);

        minMax.y += 1e-6;
        float avg = minMax.z;

        switch (METHOD) {
            case WOODCOCK:
                float meanFreePath =
                    woodcockTracking(raystart, raydir, t0, t1, hashSeed, volume, volumeParameters,
                                    transferFunction, minMax.y);
                T *= meanFreePath >= t1 ? 1f : 0f;
                break;
            case RATIO:
                T *= ratioTrackingTransmittance(raystart, raydir, t0, t1, hashSeed, volume,
                                                volumeParameters, transferFunction,
                                                minMax.y);
                break;
            case RESIDUALRATIO:
                T *= residualRatioTrackingTransmittance(raystart, raydir, t0, t1, hashSeed,
                                                        volume, volumeParameters, transferFunction,
                                                        minMax.y, avg);
                break;
        }
    }

    return T;
}

float transmittance(int METHOD, vec3 raystart, vec3 raydir, float tStart, float tEnd,
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