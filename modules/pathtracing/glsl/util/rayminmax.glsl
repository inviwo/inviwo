#include "uniformgrid.glsl"

vec2 rayMinMax(vec3 pos, vec3 dir, float t0, float t1, sampler3D opacity,
               VolumeParameters opacityParameters, vec3 cellDim, sampler3D volumeData,
               VolumeParameters volumeParameters, sampler2D tf) {

    // TODO: Convert to ivec3 through ceiling
    mat4 m = volumeParameters.textureToIndex;

    // Compare entry exit to x1 x2

    vec3 x1 = transformPoint(m, pos + t0 * dir) + 0.5f;
    vec3 x2 = transformPoint(m, pos + t1 * dir) + 0.5f;

    ivec3 cellCoord, cellCoordEnd, di;
    vec3 dt, deltatx;
    setupUniformGridTraversal(x1, x2, cellDim, ivec3(opacityParameters.dimensions), cellCoord,
                              cellCoordEnd, di, dt, deltatx);

    bool continueTraversal = true;

    vec3 minMax = getVoxel(opacity, opacityParameters, cellCoord).xyz;

    int c = 0;

    continueTraversal = stepToNextCellNoHit(deltatx, di, cellCoordEnd, dt, cellCoord);

    while (continueTraversal) {

        vec3 gridMinMaxVal = getNormalizedVoxel(opacity, opacityParameters, cellCoord).xyz;

        minMax =
            vec3(min(minMax.x, gridMinMaxVal.x), max(minMax.y, gridMinMaxVal.y), gridMinMaxVal.z);

        continueTraversal = stepToNextCellNoHit(deltatx, di, cellCoordEnd, dt, cellCoord);

        c += 1;
        if (c > 2e16) {
            continueTraversal = false;
            return minMax.xy;
        }
    }

    return minMax.xy;
}

vec3 rayMinMaxAvg(vec3 pos, vec3 dir, float t0, float t1, sampler3D opacity,
                  VolumeParameters opacityParameters, VolumeParameters volumeParameters) {

    vec3 cellDim = volumeParameters.dimensions * opacityParameters.reciprocalDimensions;

    if (t1 <= t0) {
        return vec3(0.f);
    }

    vec3 x_1 = pos + t0 * dir;
    vec3 x_2 = pos + t1 * dir;
    mat4 m = volumeParameters.textureToIndex;

    vec3 x1 = (m * vec4(x_1, 1.f)).xyz + 0.5f;
    vec3 x2 = (m * vec4(x_2, 1.f)).xyz + 0.5f;

    ivec3 cellCoord, cellCoordEnd, di;
    vec3 dt, deltatx;
    setupUniformGridTraversal(x1, x2, cellDim, ivec3(opacityParameters.dimensions), cellCoord,
                              cellCoordEnd, di, dt, deltatx);

    bool continueTraversal = true;

    vec3 minMax = getVoxel(opacity, opacityParameters, cellCoord).xyz;
    int c = 0;

    continueTraversal = stepToNextCellNoHit(deltatx, di, cellCoordEnd, dt, cellCoord);

    while (continueTraversal) {
        vec3 gridMinMaxVal = getVoxel(opacity, opacityParameters, cellCoord).xyz;

        minMax =
            vec3(min(minMax.x, gridMinMaxVal.x), max(minMax.y, gridMinMaxVal.y), gridMinMaxVal.z);

        continueTraversal = stepToNextCellNoHit(deltatx, di, cellCoordEnd, dt, cellCoord);

        c += 1;

        if (c > 2e16) {
            continueTraversal = false;
            return minMax;
        }
    }

    return minMax;
}

vec3 rayMinMaxAvgTest(vec3 pos, vec3 dir, float t0, float t1, sampler3D opacity,
                      VolumeParameters opacityParameters, vec3 cellDim, sampler3D volumeData,
                      VolumeParameters volumeParameters, sampler2D tf, inout vec3 auxReturn) {

    // TODO: Convert to ivec3 through ceiling
    mat4 m = volumeParameters.textureToIndex;

    // Compare entry exit to x1 x2
    // pos + t0*dir is expected to be between 0, and 1
    // x1, and x2 lie between [0, dataDim], dataDim := 150 when using boron
    vec3 x1 = transformPoint(m, pos + t0 * dir) + 0.5f;
    vec3 x2 = transformPoint(m, pos + t1 * dir) + 0.5f;

    ivec3 cellCoord, cellCoordEnd, di;
    vec3 dt, deltatx;
    setupUniformGridTraversal(x1, x2, cellDim, ivec3(opacityParameters.dimensions), cellCoord,
                              cellCoordEnd, di, dt, deltatx);

    bool continueTraversal = true;

    // Debugging
    vec3 volTex = (vec3(cellCoord)) / (opacityParameters.dimensions);
    vec3 volTex05 = (vec3(cellCoord) + 0.5f) / (opacityParameters.dimensions);
    // volTex05 = (-0.5f + vec3(cellCoord) + 0.5f*cellDim)/(opacityParameters.dimensions);
    ivec3 volCoord = ivec3(transformPoint(volumeParameters.textureToIndex, volTex) + 0.5f);
    volTex =
        transformPoint(volumeParameters.indexToTexture, -1 + cellCoord * cellDim + 0.5 * cellDim);

    vec4 volSample = getNormalizedVoxel(volumeData, volumeParameters, volTex);

    float opacityTest = applyTF(tf, volSample).a;
    vec3 minMax = getVoxel(opacity, opacityParameters, cellCoord).xyz;

    // Test minMax validity
    if (opacityTest > minMax.y) {
        auxReturn.x += 1;
        // abs(opacityTest - minMax.y);
    }

    int c = 0;

    continueTraversal = stepToNextCellNoHit(deltatx, di, cellCoordEnd, dt, cellCoord);

    while (continueTraversal) {
        // Debugging
        volTex = (vec3(cellCoord)) / (opacityParameters.dimensions);
        volTex05 = (vec3(cellCoord) + 0.5f) / (opacityParameters.dimensions);

        // volTex05 = (-0.5f + vec3(cellCoord) + 0.5f*cellDim)/(opacityParameters.dimensions);

        volCoord = ivec3(transformPoint(volumeParameters.textureToIndex, volTex) + 0.5f);

        volTex = transformPoint(volumeParameters.indexToTexture,
                                -1 + cellCoord * cellDim + 0.5 * cellDim);

        volSample = getNormalizedVoxel(volumeData, volumeParameters, volTex);
        opacityTest = applyTF(tf, volSample).a;

        vec3 gridMinMaxVal = getNormalizedVoxel(opacity, opacityParameters, cellCoord).xyz;

        // Test minMax validity
        if (opacityTest > gridMinMaxVal.y) {
            auxReturn.x += 1;
            // abs(opacityTest - gridMinMaxVal.y);
        }

        minMax =
            vec3(min(minMax.x, gridMinMaxVal.x), max(minMax.y, gridMinMaxVal.y), gridMinMaxVal.z);

        continueTraversal = stepToNextCellNoHit(deltatx, di, cellCoordEnd, dt, cellCoord);

        c += 1;
        if (c > 2e16) {
            continueTraversal = false;
            return minMax;
        }
    }

    return minMax;
}