#include "uniformgrid.glsl"

vec2 rayMinMax(vec3 pos, vec3 dir, float t0, float t1, sampler3D opacity,
               VolumeParameters opacityParameters, VolumeParameters dataParameters) {
                
    vec3 cellDim = dataParameters.dimensions * opacityParameters.reciprocalDimensions;

    vec3 x_1 = pos + t0 * dir;
    vec3 x_2 = pos + t1 * dir;
    mat4 m = dataParameters.textureToIndex;

    vec3 x1 = (m*vec4(x_1, 1.f)).xyz + 0.5f;
    vec3 x2 = (m*vec4(x_2, 1.f)).xyz + 0.5f;

    ivec3 cellCoord, cellCoordEnd, di;
    vec3 dt, deltatx;
    setupUniformGridTraversal(x1, x2, cellDim, ivec3(dataParameters.dimensions), cellCoord,
                              cellCoordEnd, di, dt, deltatx);

    bool continueTraversal = true;

    vec3 minMax = getNormalizedVoxel(opacity, opacityParameters, cellCoord).xyz;
    int c = 0;

    continueTraversal = stepToNextCellNoHit(deltatx, di, cellCoordEnd, dt, cellCoord);

    while (continueTraversal) {
        vec3 gridMinMaxVal = getNormalizedVoxel(opacity, opacityParameters, cellCoord).xyz;
        
        minMax =
            vec3(min(minMax.x, gridMinMaxVal.x), max(minMax.y, gridMinMaxVal.y), gridMinMaxVal.z);

        continueTraversal = stepToNextCellNoHit(deltatx, di, cellCoordEnd, dt, cellCoord);
        
        c += 1;
        
        if(c > 2e16) {
            continueTraversal = false;
            return minMax.xy;
        }
        
    }

    return minMax.xy;
}