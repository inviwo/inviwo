#include "random.glsl"
#include "utils/shading.glsl"
#include "utils/intersection.glsl"
#include "utils/gradients.glsl"
#include "util/rayminmax.glsl"
#include "distributions.glsl"

// Method Name pseudo enum
const int WOODCOCK = 0;
const int RATIO = 1;
const int RESIDUALRATIO = 2;
const int POISSONRATIO = 3;
const int POISSONRESIDUAL = 4;

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

// Poisson Transmittance Trackers
// Credit Jönsson, 2020 Direct Transmittance Estimation Using Approximated Taylor Expansion
float poissonTrackingTransmittance(vec3 raystart, vec3 raydir, float tStart, float tEnd,
                                   inout uint hashSeed, sampler3D volume,
                                   VolumeParameters volumeParameters, sampler2D transferFunction,
                                   float opacityUpperbound) {

    if (opacityUpperbound < 2e-6) {
        return 1.f;
    }

    float invMaxExtinction = 1.f / opacityUpperbound;  // should it be extinction or opacity
                                                       // (extinctionToOpacity(extinction)) ?
    float d = (tEnd - tStart);
    float T = 1.f;  // Transmittance

    int k = poisson_uni(hashSeed, d * (opacityToExtinction(opacityUpperbound)));

// if not defined.
#ifndef POISSON_SORTED_TRAVERSAL

    for (int i = 0; i < k; i++) {
        float t = tStart + randomize(hashSeed) * d;
        vec3 samplePos = raystart + t * raydir;
        vec4 volumeSample = getNormalizedVoxel(volume, volumeParameters, samplePos);
        float opacity = applyTF(transferFunction, volumeSample).a;

        T *= (1.f - opacity * invMaxExtinction);
    }

#else

#endif

    return T;
}

float poissonResidualTrackingTransmittance(vec3 raystart, vec3 raydir, float tStart, float tEnd,
                                           inout uint hashSeed, sampler3D volume,
                                           VolumeParameters volumeParameters,
                                           sampler2D transferFunction, float opacityUpperbound,
                                           float opacityControl) {

    if (opacityUpperbound < 2e-6) {
        return 1.f;
    }

    float invMaxExtinction = 1.f / opacityUpperbound;  // should it be extinction or opacity
                                                       // (extinctionToOpacity(extinction)) ?
    float d = (tEnd - tStart);
    float Tc = exp(-opacityToExtinction(opacityControl) * d);
    float Tr = 1.f;  // Transmittance

    int k = poisson_uni(hashSeed, d * (opacityToExtinction(opacityUpperbound)));

    // if not defined.
#ifndef POISSON_SORTED_TRAVERSAL

    for (int i = 0; i < k; i++) {
        float t = tStart + randomize(hashSeed) * d;
        vec3 samplePos = raystart + t * raydir;
        vec4 volumeSample = getNormalizedVoxel(volume, volumeParameters, samplePos);
        float opacity = applyTF(transferFunction, volumeSample).a;

        Tr *= (1.f - (opacity - opacityControl) * invMaxExtinction);
    }

#else
    // merge sort and use that for t

#endif

    return Tr * Tc;
}

float independentMultiPoissonTrackingTransmittance(
    vec3 raystart, vec3 raydir, float tStart, float tEnd, inout uint hashSeed, sampler3D volume,
    VolumeParameters volumeParameters, sampler2D transferFunction, float opacityUpperbound) {
    if (opacityUpperbound < 2e-6) {
        return 1.f;
    }

#ifdef POISSON_TRACKING_N
    int N = POISSON_TRACKING_N;
#else
    int N = 16;
#endif

    float opacityControl = 0;
    float d = (tEnd - tStart);

    for (int i = 0; i < N; i++) {
        float t = tStart + randomize(hashSeed) * d;
        vec3 samplePos = raystart + t * raydir;
        vec4 volumeSample = getNormalizedVoxel(volume, volumeParameters, samplePos);
        opacityControl = applyTF(transferFunction, volumeSample).a;
    }
    opacityControl /= float(N);
    return poissonResidualTrackingTransmittance(raystart, raydir, tStart, tEnd, hashSeed, volume,
                                                volumeParameters, transferFunction,
                                                opacityUpperbound, opacityControl);
}

float dependentMultiPoissonTrackingTransmittance(vec3 raystart, vec3 raydir, float tStart,
                                                 float tEnd, inout uint hashSeed, sampler3D volume,
                                                 VolumeParameters volumeParameters,
                                                 sampler2D transferFunction,
                                                 float opacityUpperbound) {
    if (opacityUpperbound < 2e-6) {
        return 1.f;
    }
    float d = (tEnd - tStart);
    int N = poisson_uni(hashSeed, d * opacityToExtinction(opacityUpperbound));

    float opacityControl = 0;

#ifndef POISSON_TRACKING_N
    float extinctions[16];
#else
    float extinctions[POISSON_TRACKING_N];
#endif
    for (int i = 0; i < N; i++) {
        float t = tStart + randomize(hashSeed) * d;
        vec3 samplePos = raystart + t * raydir;
        vec4 volumeSample = getNormalizedVoxel(volume, volumeParameters, samplePos);
        float extinction = applyTF(transferFunction, volumeSample).a;
        opacityControl += extinction;
        // and if POISSON_TRACKING_N isnt defined?

        if (i < 16) extinctions[i] = extinction;
    }

    if (N > 0) opacityControl /= float(N);

    float invMaxExtinction = 1.f / (opacityUpperbound - opacityControl);
    float Tc = exp(-d * opacityToExtinction(opacityControl));
    float Tr = 1.f;

    int k = poisson_uni(hashSeed, d * (opacityToExtinction(opacityUpperbound - opacityControl)));

    // why 16? why is POISSON_TRACKING_N = 16? optimal after testing?
    // 16 is defualt, but the processor can rewrite it when building the shader, if requested.
    int kCached = min(min(k, N), 16);
    int ki = 0;

    while (ki < kCached && ki < k) {
        Tr *= (1.f - (extinctions[ki] - opacityControl) * invMaxExtinction);
        ki++;
    }

    for (; ki < k; ki++) {
        float t = tStart + randomize(hashSeed) * d;
        vec3 samplePos = raystart + t * raydir;
        vec4 volumeSample = getNormalizedVoxel(volume, volumeParameters, samplePos);
        float opacity = applyTF(transferFunction, volumeSample).a;

        Tr *= (1.f - (opacity - opacityControl) * invMaxExtinction);
    }

    return 1.f;
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
    setupUniformGridTraversal(x1, x2, cellDim, ivec3(volumeParameters.dimensions), cellCoord,
                              cellCoordEnd, di, dt, deltatx);
    float dirLen = tEnd - tStart;
    float t = 0.f;

    bool continueTraversal = true;
    float T = 1.f;
    while (continueTraversal) {
        vec3 minMax = getNormalizedVoxel(opacity, opacityParameters, cellCoord).xyz;
        float tPrev = t;
        continueTraversal = stepToNextCellHit(deltatx, di, cellCoordEnd, dt, cellCoord, t);
        if (minMax.y <= 0) {
            continue;
        }

        float t0 = tStart + dirLen * tPrev;
        float t1 = tStart + dirLen * min(1.f, t);

        minMax.y += 1e-6;
        float avg = minMax.z;

        switch (METHOD) {
            case WOODCOCK:
                float meanFreePath = woodcockTracking(raystart, raydir, t0, t1, hashSeed, volume,
                                                      volumeParameters, transferFunction, minMax.y);
                T *= meanFreePath >= t1 ? 1f : 0f;
                break;
            case RATIO:
                T *= ratioTrackingTransmittance(raystart, raydir, t0, t1, hashSeed, volume,
                                                volumeParameters, transferFunction, minMax.y);
                break;

            case RESIDUALRATIO:
                T *= residualRatioTrackingTransmittance(raystart, raydir, t0, t1, hashSeed, volume,
                                                        volumeParameters, transferFunction,
                                                        minMax.y, avg);
                break;
            case POISSONRATIO:
                T *=  poissonTrackingTransmittance(raystart, raydir, tStart, tEnd, hashSeed,
                                                      volume, volumeParameters, transferFunction,
                                                      minMax.y);
                break;

            case POISSONRESIDUAL:
                T *=  poissonResidualTrackingTransmittance(raystart, raydir, tStart, tEnd, hashSeed,
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
        case POISSONRATIO:
                return poissonTrackingTransmittance(raystart, raydir, tStart, tEnd, hashSeed,
                                                      volume, volumeParameters, transferFunction,
                                                      opacityUpperbound);


        case POISSONRESIDUAL:
            return poissonResidualTrackingTransmittance(raystart, raydir, tStart, tEnd, hashSeed,
                                                      volume, volumeParameters, transferFunction,
                                                      opacityUpperbound, opacityControl);
    }

    return 0f;
}