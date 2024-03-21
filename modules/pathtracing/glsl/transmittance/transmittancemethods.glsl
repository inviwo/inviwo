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

float opacityToExtinction(float s_max) { return s_max * REFSAMPLINGINTERVAL; }

float woodcockTracking(vec3 raystart, vec3 raydir, float tStart, float tEnd, inout uint hashSeed,
                       sampler3D volume, VolumeParameters volumeParameters,
                       sampler2D transferFunction, float opacityUpperbound, out vec3 auxReturn) {

    float invMaxExtinction = 1.f / opacityToExtinction(opacityUpperbound);
    float invOpacitUpperbound = 1.f / opacityUpperbound;
    float t = tStart;
    float opacity;
    auxReturn = vec3(0);

    do {
        t += -log(randomize(hashSeed)) * invMaxExtinction;
        vec3 samplePos = raystart + t * raydir;
        vec4 volumeSample = getNormalizedVoxel(volume, volumeParameters, samplePos);
        opacity = applyTF(transferFunction, volumeSample).a;

        if (opacity > opacityUpperbound) {
            auxReturn.x = opacity - opacityUpperbound;
        }

    } while (randomize(hashSeed) >= opacity * invOpacitUpperbound && t <= tEnd);

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
        samplePos = samplePos + t * raydir;
        vec4 volumeSample = getNormalizedVoxel(volume, volumeParameters, samplePos);
        opacity = applyTF(transferFunction, volumeSample).a;
        if (opacity > opacityUpperbound) {
            return -10f;
        }

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

        if (opacity > opacityUpperbound) {
            return -10f;
        }

        Tr *= (1 - (opacity - opacityControl) * invOpacitUpperbound);

    } while (true);

    return clamp(Tr * Tc, 0.f, 1.f);
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

    float invMaxExtinction = 1.f / opacityToExtinction(opacityUpperbound);
    // should it be extinction or opacity
    // (extinctionToOpacity(extinction)) ?
    float d = (tEnd - tStart);
    float T = 1.f;  // Transmittance

    int k = poisson_uni(hashSeed, d * (opacityToExtinction(opacityUpperbound)));

#ifndef POISSON_SORTED_TRAVERSAL

    for (int i = 0; i < k; i++) {
        float t = tStart + randomize(hashSeed) * d;
        vec3 samplePos = raystart + t * raydir;
        vec4 volumeSample = getNormalizedVoxel(volume, volumeParameters, samplePos);
        float opacity = applyTF(transferFunction, volumeSample).a;

        if (opacity > opacityUpperbound) {
            return -10f;
        }

        T *= (1.f - opacity * invMaxExtinction);
    }

#else

#endif

    return clamp(T, 0.f, 1.f);
}

float poissonResidualTrackingTransmittance(vec3 raystart, vec3 raydir, float tStart, float tEnd,
                                           inout uint hashSeed, sampler3D volume,
                                           VolumeParameters volumeParameters,
                                           sampler2D transferFunction, float opacityUpperbound,
                                           float opacityControl) {

    if (opacityUpperbound < 2e-6) {
        return 1.f;
    }
    // was 1 / upperbound
    float invMaxExtinction = 1.f / (opacityUpperbound - opacityControl);
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

        if (opacity > opacityUpperbound) {
            return -10f;
        }

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

        if (opacity > opacityUpperbound) {
            return -10f;
        }

        Tr *= (1.f - (opacity - opacityControl) * invMaxExtinction);
    }

    return 1.f;
}

// TODO: Rename to residual, since it bases itself of the poisson residual tracker
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

        if (opacity > opacityUpperbound) {
            return -10f;
        }

        Tr *= (1.f - (opacity - opacityControl) * invMaxExtinction);
    }

    return Tr * Tc;
}

float partitionedTransmittanceTracking(int METHOD, vec3 raystart, vec3 raydir, float tStart,
                                       float tEnd, inout uint hashSeed, sampler3D volume,
                                       VolumeParameters volumeParameters,
                                       sampler2D transferFunction, sampler3D opacity,
                                       VolumeParameters opacityParameters, inout vec3 auxReturn) {

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
    setupUniformGridTraversal(x1, x2, cellDim, ivec3(opacityParameters.dimensions), cellCoord,
                              cellCoordEnd, di, dt, deltatx);
    float dirLen = tEnd - tStart;
    float t = 0.f;

    bool continueTraversal = true;
    float T = 1.f;

    while (continueTraversal) {
        // TODO: CHECK THAT ALL INSTANCES OF MINMAXOPACITY ACCESS USED getVoxel! NOT
        // getNormalizedVoxel
        //  We want the raw data. Normalizing the min max can be destructive
        vec3 minMax = getVoxel(opacity, opacityParameters, cellCoord).xyz;

        float tPrev = t;

        continueTraversal = stepToNextCellHit(deltatx, di, cellCoordEnd, dt, cellCoord, t);
        if (minMax.y <= 0) {
            //continue;
        }

        float t0 = tStart + dirLen * tPrev;
        float t1 = tStart + dirLen * min(1.f, t);

        vec4 volumeSamplet0 =
            getNormalizedVoxel(volume, volumeParameters, raystart + raydir * t0);

        vec4 volumeSamplet1 =
            getNormalizedVoxel(volume, volumeParameters, raystart + raydir * t1);


        minMax.y += 1e-6;
        float avg = minMax.z;

        float op = applyTF(transferFunction, volumeSamplet1).a;

        // We seemingly get this clause when sampling close to the edge of a cell in opacity
        if (op > minMax.y) {

            auxReturn += vec3(0,0,abs(op - minMax.y));
        }

        // Should be METHOD, but the uniform is not behaving
        // Check that sampled opacity
        float T_ = 0;
        vec3 auxReturnSub = vec3(0);
        switch (METHOD) {
            case 0 :
            float meanFreePath = woodcockTracking(raystart, raydir, t0, t1, hashSeed, volume,
                                                  volumeParameters, transferFunction, minMax.y, auxReturnSub);

            if(auxReturnSub.x > 0) {
                auxReturn += vec3(auxReturnSub.x,0,0);
                
            } else {
                
            }
            T *= meanFreePath >= t1 ? 1f : 0f;       
            
            break;
            case 1:
            T_ = ratioTrackingTransmittance(raystart, raydir, t0, t1, hashSeed, volume,
                                            volumeParameters, transferFunction, minMax.y);

            if(T_ < 0) {
                auxReturn += vec3(1,0,0);
            } else {
                T *= T_;
            }
            
            break;
            case 2:
            T_ = residualRatioTrackingTransmittance(raystart, raydir, t0, t1, hashSeed, volume,
                                                    volumeParameters, transferFunction, minMax.y,
                                                    avg);
            if(T_ < 0) {
                auxReturn += vec3(1,0,0);
            } else {
                T *= T_;
            }

            break;
            case 3:
            T_ = poissonTrackingTransmittance(raystart, raydir, tStart, tEnd, hashSeed, volume,
                                              volumeParameters, transferFunction, minMax.y);

            if(T_ < 0) {
                auxReturn += vec3(1,0,0);
            } else {
                T *= T_;
            }

            break;
            case 4:
            T_ = poissonResidualTrackingTransmittance(raystart, raydir, tStart, tEnd, hashSeed,
                                                      volume, volumeParameters, transferFunction,
                                                      minMax.y, avg);

            if(T_ < 0) {
                auxReturn += vec3(1,0,0);
            } else {
                T *= T_;
            }

            case 5:
            T_ = independentMultiPoissonTrackingTransmittance(raystart, raydir, tStart, tEnd,
                                                              hashSeed, volume, volumeParameters,
                                                              transferFunction, minMax.y);

            if(T_ < 0) {
                auxReturn += vec3(1,0,0);
            } else {
                T *= T_;
            }
            
            break;
            case 6:
            T *= dependentMultiPoissonTrackingTransmittance(raystart, raydir, tStart, tEnd,
                                                            hashSeed, volume, volumeParameters,
                                                            transferFunction, minMax.y);

            if(T_ < 0) {
                auxReturn += vec3(1,0,0);
            } else {
                T *= T_;
            }

            break;
            case 7:
            T *= geometricTrackingTransmittance(raystart, raydir, tStart, tEnd, hashSeed, volume,
                                                volumeParameters, transferFunction, minMax.y, avg);

            if(T_ < 0) {
                auxReturn += vec3(1,0,0);
            } else {
                T *= T_;
            }
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

    vec3 auxReturn2 = vec3(0);

    switch (METHOD) {
        case WOODCOCK:
            float meanFreePath =
                woodcockTracking(raystart, raydir, tStart, tEnd, hashSeed, volume, volumeParameters,
                                 transferFunction, opacityUpperbound, auxReturn2);
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
            return poissonTrackingTransmittance(raystart, raydir, tStart, tEnd, hashSeed, volume,
                                                volumeParameters, transferFunction,
                                                opacityUpperbound);

        case POISSONRESIDUAL:
            return poissonResidualTrackingTransmittance(raystart, raydir, tStart, tEnd, hashSeed,
                                                        volume, volumeParameters, transferFunction,
                                                        opacityUpperbound, opacityControl);
        case INDEPENDENTPOISSON:
            return independentMultiPoissonTrackingTransmittance(
                raystart, raydir, tStart, tEnd, hashSeed, volume, volumeParameters,
                transferFunction, opacityUpperbound);
        case DEPENDENTPOISSON:
            return dependentMultiPoissonTrackingTransmittance(raystart, raydir, tStart, tEnd,
                                                              hashSeed, volume, volumeParameters,
                                                              transferFunction, opacityUpperbound);

        case GEOMETRICRESIDUAL:
            return geometricTrackingTransmittance(raystart, raydir, tStart, tEnd, hashSeed, volume,
                                                  volumeParameters, transferFunction,
                                                  opacityUpperbound, opacityControl);
    }

    return 0f;
}

float partitionedTransmittanceTesting(int METHOD, vec3 raystart, vec3 raydir, float tStart, float tEnd,
                           inout uint hashSeed, sampler3D volume, VolumeParameters volumeParameters,
                           sampler2D transferFunction, sampler3D opacity,
                           VolumeParameters opacityParameters, inout vec3 auxReturn) {


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
    setupUniformGridTraversal(x1, x2, cellDim, ivec3(opacityParameters.dimensions), cellCoord,
                              cellCoordEnd, di, dt, deltatx);
    float dirLen = tEnd - tStart;
    float t = 0.f;

    bool continueTraversal = true;
    float T = 1.f;

    while (continueTraversal) {
        // TODO: CHECK THAT ALL INSTANCES OF MINMAXOPACITY ACCESS USED getVoxel! NOT
        // getNormalizedVoxel
        //  We want the raw data. Normalizing the min max can be destructive
        vec3 minMax = getVoxel(opacity, opacityParameters, cellCoord).xyz;

        float tPrev = t;

        continueTraversal = stepToNextCellHit(deltatx, di, cellCoordEnd, dt, cellCoord, t);
        if (minMax.y <= 0) {
            //continue;
        }

        float t0 = tStart + dirLen * tPrev;
        float t1 = tStart + dirLen * min(1.f, t);

        vec4 volumeSamplet0 =
            getNormalizedVoxel(volume, volumeParameters, raystart + raydir * t0);

        vec4 volumeSamplet1 =
            getNormalizedVoxel(volume, volumeParameters, raystart + raydir * t1);

        vec4 volumeSamplet05 =
            getNormalizedVoxel(volume, volumeParameters, raystart + raydir * ((t1 + t0) * 0.5));


        minMax.y += 1e-6;
        float avg = minMax.z;

        float op = applyTF(transferFunction, volumeSamplet1).a;

        // We seemingly get this clause when sampling close to the edge of a cell in opacity
        if (op > minMax.y) {

            auxReturn += vec3(0,0,abs(op - minMax.y));
        }

        // Should be METHOD, but the uniform is not behaving
        // Check that sampled opacity
        float T_ = 0;
        vec3 auxReturnSub = vec3(0);
        switch (METHOD) {
            case 0 :
            float meanFreePath = woodcockTracking(raystart, raydir, t0, t1, hashSeed, volume,
                                                  volumeParameters, transferFunction, minMax.y, auxReturnSub);

            if(auxReturnSub.x > 0) {
                auxReturn += vec3(auxReturnSub.x,0,0);
                
            } else {
                
            }
            T *= meanFreePath >= t1 ? 1f : 0f;       
            
            break;
            case 1:
            T_ = ratioTrackingTransmittance(raystart, raydir, t0, t1, hashSeed, volume,
                                            volumeParameters, transferFunction, minMax.y);

            if(T_ < 0) {
                auxReturn += vec3(1,0,0);
            } else {
                T *= T_;
            }
            
            break;
            case 2:
            T_ = residualRatioTrackingTransmittance(raystart, raydir, t0, t1, hashSeed, volume,
                                                    volumeParameters, transferFunction, minMax.y,
                                                    avg);
            if(T_ < 0) {
                auxReturn += vec3(1,0,0);
            } else {
                T *= T_;
            }

            break;
            case 3:
            T_ = poissonTrackingTransmittance(raystart, raydir, tStart, tEnd, hashSeed, volume,
                                              volumeParameters, transferFunction, minMax.y);

            if(T_ < 0) {
                auxReturn += vec3(1,0,0);
            } else {
                T *= T_;
            }

            break;
            case 4:
            T_ = poissonResidualTrackingTransmittance(raystart, raydir, tStart, tEnd, hashSeed,
                                                      volume, volumeParameters, transferFunction,
                                                      minMax.y, avg);

            if(T_ < 0) {
                auxReturn += vec3(1,0,0);
            } else {
                T *= T_;
            }

            case 5:
            T_ = independentMultiPoissonTrackingTransmittance(raystart, raydir, tStart, tEnd,
                                                              hashSeed, volume, volumeParameters,
                                                              transferFunction, minMax.y);

            if(T_ < 0) {
                auxReturn += vec3(1,0,0);
            } else {
                T *= T_;
            }
            
            break;
            case 6:
            T *= dependentMultiPoissonTrackingTransmittance(raystart, raydir, tStart, tEnd,
                                                            hashSeed, volume, volumeParameters,
                                                            transferFunction, minMax.y);

            if(T_ < 0) {
                auxReturn += vec3(1,0,0);
            } else {
                T *= T_;
            }

            break;
            case 7:
            T *= geometricTrackingTransmittance(raystart, raydir, tStart, tEnd, hashSeed, volume,
                                                volumeParameters, transferFunction, minMax.y, avg);

            if(T_ < 0) {
                auxReturn += vec3(1,0,0);
            } else {
                T *= T_;
            }
            break;
        }
    }

    return T;
}