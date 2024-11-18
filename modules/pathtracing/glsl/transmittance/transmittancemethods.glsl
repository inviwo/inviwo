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
const int INDEPENDENTPOISSON = 5;
const int DEPENDENTPOISSON = 6;
const int GEOMETRICRESIDUAL = 7;

#define REFSAMPLINGINTERVAL 150.0

#define upperBoundMultiplier 3.0

/*
    Note: The Residual methods fail when using opacity control directly from the uniform grid for a 'high frequency' transmittance function.
    Independent Multiple poisson tracking, while using poisson residual ratio tracking, does not suffer from the obvious artifacts of the acceleration grid.  
*/
/*
    In a perfect world, clamps on the return would not exist, but this is not a perfect world.
*/
float opacityToExtinction(float s_max) { return s_max * REFSAMPLINGINTERVAL; }

float woodcockTracking(vec3 raystart, vec3 raydir, float tStart, float tEnd, inout uint hashSeed,
                       sampler3D volume, VolumeParameters volumeParameters,
                       sampler2D transferFunction, float opacityUpperbound) {
    float invMaxExtinction = 1.f / opacityToExtinction(opacityUpperbound);
    float invOpacitUpperbound = 1.f / opacityUpperbound;
    float t = tStart;
    float opacity;

    do {
        t += -log(randomize(hashSeed)) * invMaxExtinction;
        vec3 samplePos = raystart + t * raydir;

        // NOTE: Simply to stop samplePos fromo falling outside of relevant opacity voxel, where
        //       opacityUpperbound no longer is correct when doing opacity testing. 'Should be'
        //       logically equivalent to having
        //       && (t <= tEnd) in the while clause.
        if (t > tEnd) {
            break;
        }

        vec4 volumeSample = getNormalizedVoxel(volume, volumeParameters, samplePos);
        opacity = applyTF(transferFunction, volumeSample).a;


    } while ((randomize(hashSeed) >= opacity * invOpacitUpperbound));

    return t;
}

float ratioTrackingTransmittance(vec3 raystart, vec3 raydir, float tStart, float tEnd,
                                 inout uint hashSeed, sampler3D volume,
                                 VolumeParameters volumeParameters, sampler2D transferFunction,
                                 float opacityUpperbound) {

    if (opacityUpperbound < 2e-6) {
        return 1.f;
    }

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
        samplePos = raystart + t * raydir;
        vec4 volumeSample = getNormalizedVoxel(volume, volumeParameters, samplePos);
        opacity = applyTF(transferFunction, volumeSample).a;

        T *= (1 - opacity * invOpacitUpperbound);

    } while (true);

    return clamp(T, 0.f, 1.f);
}

float residualRatioTrackingTransmittance(vec3 raystart, vec3 raydir, float tStart, float tEnd,
                                         inout uint hashSeed, sampler3D volume,
                                         VolumeParameters volumeParameters,
                                         sampler2D transferFunction, float opacityUpperbound,
                                         float opacityControl) {
    opacityUpperbound *= upperBoundMultiplier;
    if (opacityUpperbound < 2e-6) {
        return 1.f;
    }
    float invMaxExtinction = 1.f / opacityToExtinction(opacityUpperbound);
    float invOpacitUpperbound = 1.f / opacityUpperbound;
    float t = tStart;

    float Tc = exp(-opacityToExtinction(opacityControl) * (tEnd - tStart));
    float Tr = 1.f;  // transmittance

    do {
        t = t - log(randomize(hashSeed)) * invMaxExtinction;
        if (t > tEnd) {
            break;
        }
        vec3 samplePos = raystart + t * raydir;
        float volumeSample = getNormalizedVoxel(volume, volumeParameters, samplePos).x;
        float opacity = applyTF(transferFunction, volumeSample).a;

        Tr *= (1.f - (opacity - opacityControl) * invOpacitUpperbound);

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

    float invMaxExtinction = 1.f / opacityUpperbound;
    float d = (tEnd - tStart);
    float T = 1.f;  // Transmittance

    int k = poisson_uni(hashSeed, d * (opacityToExtinction(opacityUpperbound)));

    for (int i = 0; i < k; i++) {
        float t = tStart + randomize(hashSeed) * d;

        vec3 samplePos = raystart + t * raydir;
        vec4 volumeSample = getNormalizedVoxel(volume, volumeParameters, samplePos);
        float opacity = applyTF(transferFunction, volumeSample).a;

        T *= (1.f - opacity * invMaxExtinction);
    }

    return T;
}

float poissonResidualTrackingTransmittance(vec3 raystart, vec3 raydir, float tStart, float tEnd,
                                           inout uint hashSeed, sampler3D volume,
                                           VolumeParameters volumeParameters,
                                           sampler2D transferFunction, float opacityUpperbound,
                                           float opacityControl) {
    opacityUpperbound *= upperBoundMultiplier;
    if (opacityUpperbound < 2e-6) {
        return 1.f;
    }
    float invMaxExtinction = 1.f / (opacityUpperbound - opacityControl);
    float d = (tEnd - tStart);
    float Tc = exp(-opacityToExtinction(opacityControl) * d);
    float Tr = 1.f;  // Transmittance

    int k = poisson_uni(hashSeed, d * (opacityToExtinction(opacityUpperbound - opacityControl)));

    for (int i = 0; i < k; i++) {
        float t = tStart + randomize(hashSeed) * d;
        vec3 samplePos = raystart + t * raydir;
        vec4 volumeSample = getNormalizedVoxel(volume, volumeParameters, samplePos);
        float opacity = applyTF(transferFunction, volumeSample).a;

        Tr *= (1.f - (opacity - opacityControl) * invMaxExtinction);
    }

    return Tr * Tc;
}

float independentMultiPoissonTrackingTransmittance(vec3 raystart, vec3 raydir, float tStart,
                                                   float tEnd, inout uint hashSeed,
                                                   sampler3D volume,
                                                   VolumeParameters volumeParameters,
                                                   sampler2D transferFunction,
                                                   float opacityUpperbound) {

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
        opacityControl += applyTF(transferFunction, volumeSample).a;
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

    float extinctions[16];

    for (int i = 0; i < N; i++) {
        float t = tStart + randomize(hashSeed) * d;
        vec3 samplePos = raystart + t * raydir;
        vec4 volumeSample = getNormalizedVoxel(volume, volumeParameters, samplePos);
        float extinction = applyTF(transferFunction, volumeSample).a;
        opacityControl += extinction;

        if (i < 16) extinctions[i] = extinction;
    }

    if (N > 0) opacityControl /= float(N);

    float invMaxExtinction = 1.f / (opacityUpperbound - opacityControl);
    float Tc = exp(-d * opacityToExtinction(opacityControl));
    float Tr = 1.f;

    int k = poisson_uni(hashSeed, d * (opacityToExtinction(opacityUpperbound - opacityControl)));

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

    return Tc * Tr;
}

// NOTE: Non functioning
float geometricTrackingTransmittance(vec3 raystart, vec3 raydir, float tStart, float tEnd,
                                     inout uint hashSeed, sampler3D volume,
                                     VolumeParameters volumeParameters, sampler2D transferFunction,
                                     float opacityUpperbound, float opacityControl) {
    if (opacityUpperbound < 2e-6) {
        return 1.f;
    }

    float invMaxExtinction = 1.f / opacityUpperbound;  // should it be extinction or opacity
                                                       // (extinctionToOpacity(extinction)) ?
    float d = (tEnd - tStart);
    float Tc = exp(-opacityToExtinction(opacityControl) * d);
    float Tr = 1.f;  // Transmittance

    // What is the chance of succesfully traversing through from tStart to tEnd
    // set p to match lambda in variance. The variance of a poisson distribution is lambda, a
    // geometric one is (1-p)/p² so we want to set p so that d *
    // (opacityToExtinction(opacityUpperbound)) = (1-p)/p²

    int k = geomertric0_uni(hashSeed, 0.5f);

    for (int i = 0; i < k; i++) {
        float t = tStart + randomize(hashSeed) * d;
        vec3 samplePos = raystart + t * raydir;
        vec4 volumeSample = getNormalizedVoxel(volume, volumeParameters, samplePos);
        float opacity = applyTF(transferFunction, volumeSample).a;

        Tr *= (1.f - (opacity - opacityControl) * invMaxExtinction);
    }

    return Tr * Tc;
}
