#include "utils/shading.glsl"
#include "utils/intersection.glsl"
#include "utils/gradients.glsl"
#include "util/rayminmax.glsl"
#include "transmittance/transmittancemethods.glsl"

float partitionedTransmittanceTracking(int METHOD, vec3 raystart, vec3 raydir, float tStart,
                                       float tEnd, inout uint hashSeed, sampler3D volume,
                                       VolumeParameters volumeParameters,
                                       sampler2D transferFunction, sampler3D opacity,
                                       VolumeParameters opacityParameters,
                                       sampler3D avgOpacity, vec3 cellDim) {

    mat4 volumeTexToIndMat = volumeParameters.textureToIndex;

    vec3 x1 = (volumeTexToIndMat * vec4(raystart + tStart * raydir, 1.0f)).xyz + 0.5f;
    vec3 x2 = (volumeTexToIndMat * vec4(raystart + tEnd * raydir, 1.0f)).xyz + 0.5f;

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
        vec3 minMax = getVoxel(opacity, opacityParameters, cellCoord).xyz;

        float tPrev = t;

        continueTraversal = stepToNextCellHit(deltatx, di, cellCoordEnd, dt, cellCoord, t);
        if (minMax.y <= 0) {
            continue;
        }

        float t0 = tStart + dirLen * tPrev;
        float t1 = tStart + dirLen * min(1.f, t);

        minMax.y += 1e-6;
        float avg = getVoxel(avgOpacity, opacityParameters, raystart + raydir*(t0 + t1)*0.5).x;
        // avg = (minMax.y + minMax.x)*0.5f;
        // avg = 0.5;
        switch (METHOD) {

            case 0:
                float meanFreePath = woodcockTracking(raystart, raydir, t0, t1, hashSeed, volume,
                                                      volumeParameters, transferFunction, minMax.y);

                T *= meanFreePath >= t1 ? 1f : 0f;
                break;
            case 1:
                T *= ratioTrackingTransmittance(raystart, raydir, t0, t1, hashSeed, volume,
                                                volumeParameters, transferFunction, minMax.y);
                break;
            case 2:
                T *= residualRatioTrackingTransmittance(raystart, raydir, t0, t1, hashSeed, volume,
                                                        volumeParameters, transferFunction,
                                                        minMax.y, avg);
                break;
            case 3:
                T *= poissonTrackingTransmittance(raystart, raydir, t0, t1, hashSeed, volume,
                                                  volumeParameters, transferFunction, minMax.y);

                break;
            case 4:
                T *= poissonResidualTrackingTransmittance(raystart, raydir, t0, t1, hashSeed,
                                                          volume, volumeParameters,
                                                          transferFunction, minMax.y, avg);
                break;
            case 5:
                T *= independentMultiPoissonTrackingTransmittance(
                    raystart, raydir, t0, t1, hashSeed, volume, volumeParameters, transferFunction,
                    minMax.y);
                break;
            case 6:
                T *= dependentMultiPoissonTrackingTransmittance(raystart, raydir, t0, t1, hashSeed,
                                                                volume, volumeParameters,
                                                                transferFunction, minMax.y);
                break;
        }
    }

    return T;
}

float transmittance(int METHOD, vec3 raystart, vec3 raydir, float tStart, float tEnd,
                    inout uint hashSeed, sampler3D volume, VolumeParameters volumeParameters,
                    sampler2D transferFunction) {

    float opacityUpperbound = 1.0f;
    opacityUpperbound += 1e-6;
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
    }

    return 0f;
}

float partitionedTransmittanceTesting(int METHOD, vec3 raystart, vec3 raydir, float tStart,
                                      float tEnd, inout uint hashSeed, sampler3D volume,
                                      VolumeParameters volumeParameters, sampler2D transferFunction,
                                      sampler3D opacity, VolumeParameters opacityParameters,
                                      vec3 cellDim) {

    mat4 volumeTexToIndMat = volumeParameters.textureToIndex;

    vec3 x1 = (volumeTexToIndMat * vec4(raystart + tStart * raydir, 1.0f)).xyz + 0.5f;
    vec3 x2 = (volumeTexToIndMat * vec4(raystart + tEnd * raydir, 1.0f)).xyz + 0.5f;

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
        vec3 minMax = getVoxel(opacity, opacityParameters, cellCoord).xyz;

        float tPrev = t;

        continueTraversal = stepToNextCellHit(deltatx, di, cellCoordEnd, dt, cellCoord, t);
        if (minMax.y <= 0) {
            continue;
        }

        float t0 = tStart + dirLen * tPrev;
        float t1 = tStart + dirLen * min(1.f, t);

        vec4 volumeSamplet05 =
            getNormalizedVoxel(volume, volumeParameters, raystart + raydir * ((t1 + t0) * 0.5));

        // minMax.y += 1e-6;
        float avg = minMax.z;

        float op = applyTF(transferFunction, volumeSamplet05).a;

        switch (METHOD) {

            case 0:
                float meanFreePath = woodcockTracking(raystart, raydir, t0, t1, hashSeed, volume,
                                                      volumeParameters, transferFunction, minMax.y);

                T *= meanFreePath > t1 ? 1f : 0f;
                break;
            case 1:
                T *= ratioTrackingTransmittance(raystart, raydir, t0, t1, hashSeed, volume,
                                                volumeParameters, transferFunction, minMax.y);
                break;
            case 2:
                T *= residualRatioTrackingTransmittance(raystart, raydir, t0, t1, hashSeed, volume,
                                                        volumeParameters, transferFunction,
                                                        minMax.y, avg);
                break;
            case 3:
                T *= poissonTrackingTransmittance(raystart, raydir, t0, t1, hashSeed, volume,
                                                  volumeParameters, transferFunction, minMax.y);
                break;
            case 4:
                T *= poissonResidualTrackingTransmittance(raystart, raydir, t0, t1, hashSeed,
                                                          volume, volumeParameters,
                                                          transferFunction, minMax.y, avg);
                break;
            case 5:
                T *= independentMultiPoissonTrackingTransmittance(
                    raystart, raydir, t0, t1, hashSeed, volume, volumeParameters, transferFunction,
                    minMax.y);
                break;
            case 6:
                T *= dependentMultiPoissonTrackingTransmittance(raystart, raydir, t0, t1, hashSeed,
                                                                volume, volumeParameters,
                                                                transferFunction, minMax.y);
                break;
        }
    }

    return T;
}
