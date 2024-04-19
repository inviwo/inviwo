#include "utils/shading.glsl"
#include "utils/intersection.glsl"
#include "utils/gradients.glsl"
#include "util/rayminmax.glsl"
#include "transmittance/transmittancemethods.glsl"

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
        vec3 minMax = getVoxel(opacity, opacityParameters, cellCoord).xyz;

        float tPrev = t;

        continueTraversal = stepToNextCellHit(deltatx, di, cellCoordEnd, dt, cellCoord, t);
        if (minMax.y <= 0) {
            continue;
        }

        float t0 = tStart + dirLen * tPrev;
        float t1 = tStart + dirLen * min(1.f, t);

        minMax.y += 1e-6;
        float avg = minMax.z;
        float T_ = 0;
        vec3 auxReturnSub = vec3(0);
        switch (METHOD) {
    
            case 0 :
            float meanFreePath = woodcockTracking(raystart, raydir, t0, t1, hashSeed, volume,
                                                  volumeParameters, transferFunction, minMax.y, auxReturnSub);

            if(auxReturnSub != vec3(0)) {
                auxReturn += vec3(auxReturnSub.x,0,0);
                
            }
            T *= meanFreePath >= t1 ? 1f : 0f;
            
            break;
            case 1:
            T_ = ratioTrackingTransmittance(raystart, raydir, t0, t1, hashSeed, volume,
                                            volumeParameters, transferFunction, minMax.y, auxReturnSub);

            if(auxReturnSub != vec3(0)) {
                auxReturn += auxReturnSub;
            }
            T *= T_;
            
            break;
            case 2:
            T_ = residualRatioTrackingTransmittance(raystart, raydir, t0, t1, hashSeed, volume,
                                                    volumeParameters, transferFunction, minMax.y,
                                                    avg, auxReturnSub);
            if(auxReturnSub != vec3(0)) {
                auxReturn += auxReturnSub;
            }
            T *= T_;

            break;
            case 3:
            T_ = poissonTrackingTransmittance(raystart, raydir, t0, t1, hashSeed, volume,
                                              volumeParameters, transferFunction, minMax.y, auxReturnSub);

            if(auxReturnSub != vec3(0)) {
                auxReturn += auxReturnSub;
            }
            T *= T_;

            break;
            case 4:
            T_ = poissonResidualTrackingTransmittance(raystart, raydir, t0, t1, hashSeed,
                                                      volume, volumeParameters, transferFunction,
                                                      minMax.y, avg, auxReturnSub);

            if(auxReturnSub != vec3(0)) {
                auxReturn += auxReturnSub;
            }
            T *= T_;
            break;
            case 5:
            T_ = independentMultiPoissonTrackingTransmittance(raystart, raydir, t0, t1,
                                                              hashSeed, volume, volumeParameters,
                                                              transferFunction, minMax.y, auxReturnSub);

            if(auxReturnSub != vec3(0)) {
                auxReturn += auxReturnSub;
            } 
            T *= T_;
            break;
            case 6:
            T_ = dependentMultiPoissonTrackingTransmittance(raystart, raydir, t0, t1,
                                                            hashSeed, volume, volumeParameters,
                                                            transferFunction, minMax.y, auxReturnSub);

            if(auxReturnSub != vec3(0)) {
                auxReturn += auxReturnSub;
            } 
            T *= T_;

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
                                              opacityUpperbound, auxReturn2);
        case RESIDUALRATIO:
            return residualRatioTrackingTransmittance(raystart, raydir, tStart, tEnd, hashSeed,
                                                      volume, volumeParameters, transferFunction,
                                                      opacityUpperbound, opacityControl, auxReturn2);
        case POISSONRATIO:
            return poissonTrackingTransmittance(raystart, raydir, tStart, tEnd, hashSeed, volume,
                                                volumeParameters, transferFunction,
                                                opacityUpperbound, auxReturn2);

        case POISSONRESIDUAL:
            return poissonResidualTrackingTransmittance(raystart, raydir, tStart, tEnd, hashSeed,
                                                        volume, volumeParameters, transferFunction,
                                                        opacityUpperbound, opacityControl, auxReturn2);
        case INDEPENDENTPOISSON:
            return independentMultiPoissonTrackingTransmittance(
                raystart, raydir, tStart, tEnd, hashSeed, volume, volumeParameters,
                transferFunction, opacityUpperbound, auxReturn2);
        case DEPENDENTPOISSON:
            return dependentMultiPoissonTrackingTransmittance(raystart, raydir, tStart, tEnd,
                                                              hashSeed, volume, volumeParameters,
                                                              transferFunction, opacityUpperbound, auxReturn2);
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
        vec3 minMax = getVoxel(opacity, opacityParameters, cellCoord).xyz;

        float tPrev = t;

        continueTraversal = stepToNextCellHit(deltatx, di, cellCoordEnd, dt, cellCoord, t);
        if (minMax.y <= 0) {
            continue;
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

        float op = applyTF(transferFunction, volumeSamplet05).a;

        // We seemingly get this clause when sampling close to the edge of a cell in opacity
        if (op > minMax.y) {

            auxReturn += vec3(0,0,abs(op - minMax.y));
        }
        float T_ = 0;
        vec3 auxReturnSub = vec3(0);
        switch (METHOD) {
            
            case 0 :
            float meanFreePath = woodcockTracking(raystart, raydir, t0, t1, hashSeed, volume,
                                                  volumeParameters, transferFunction, minMax.y, auxReturnSub);

            if(auxReturnSub != vec3(0)) {
                auxReturn += vec3(auxReturnSub.x,0,0);
                
            }
            T *= meanFreePath >= t1 ? 1f : 0f;
            
            break;
            case 1:
            T_ = ratioTrackingTransmittance(raystart, raydir, t0, t1, hashSeed, volume,
                                            volumeParameters, transferFunction, minMax.y, auxReturnSub);

            if(auxReturnSub != vec3(0)) {
                auxReturn += vec3(0,0,0);
            }
            T *= T_;
            
            break;
            case 2:
            T_ = residualRatioTrackingTransmittance(raystart, raydir, t0, t1, hashSeed, volume,
                                                    volumeParameters, transferFunction, minMax.y,
                                                    avg, auxReturnSub);
            if(auxReturnSub != vec3(0)) {
                auxReturn += vec3(auxReturnSub.x,0,0);
            }
            T *= T_;

            break;
            case 3:
            T_ = poissonTrackingTransmittance(raystart, raydir, t0, t1, hashSeed, volume,
                                              volumeParameters, transferFunction, minMax.y, auxReturnSub);

            if(auxReturnSub != vec3(0)) {
                auxReturn += vec3(auxReturnSub.x,0,0);
            }
            T *= T_;

            break;
            case 4:
            T_ = poissonResidualTrackingTransmittance(raystart, raydir, t0, t1, hashSeed,
                                                      volume, volumeParameters, transferFunction,
                                                      minMax.y, avg, auxReturnSub);

            if(auxReturnSub != vec3(0)) {
                auxReturn += vec3(auxReturnSub.x,0,0);
            }
            T *= T_;
            break;
            case 5:
            T_ = independentMultiPoissonTrackingTransmittance(raystart, raydir, t0, t1,
                                                              hashSeed, volume, volumeParameters,
                                                              transferFunction, minMax.y, auxReturnSub);

            if(auxReturnSub != vec3(0)) {
                auxReturn += vec3(auxReturnSub.x,0,0);
            } 
            T *= T_;
            break;
            case 6:
            T_ = dependentMultiPoissonTrackingTransmittance(raystart, raydir, t0, t1,
                                                            hashSeed, volume, volumeParameters,
                                                            transferFunction, minMax.y, auxReturnSub);

            if(auxReturnSub != vec3(0)) {
                auxReturn += vec3(auxReturnSub.x,auxReturnSub.y,auxReturnSub.z);
            } 
            T *= T_;

            break;
        }
    }

    return T;
}