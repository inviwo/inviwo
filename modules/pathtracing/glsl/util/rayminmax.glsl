#include "uniformgrid.glsl"

vec2 rayMinMax(vec3 pos, vec3 dir, float t0, float t1, sampler3D opacity,
               VolumeParameters opacityParameters, VolumeParameters dataParameters, bool partitionedTransmittance) {

    // problem that needs solving: can we get the cell dimension from the volume?
    // From the data volume and the opacity volume we can solve for region size. data.dimensions /
    // minMaxOpacity.dimensions I could not find exactly where UGrid::cellDimensions is set in the
    // opacity processor, it is defaulted to 1,1,1 But I'm thinking 'How could it be anything
    // other?'

    if(!partitionedTransmittance) {
        return vec2(0, 1f);
    }

    vec3 cellDim = dataParameters.dimensions * opacityParameters.reciprocalDimensions;

    // cast down texttoindex or cast up pos + t0*dir
    vec3 x1 = (opacityParameters.textureToIndex * vec4(pos + t0 * dir, 1.f)).xyz + 0.5f;
    vec3 x2 = (opacityParameters.textureToIndex * vec4(pos + t1 * dir, 1.f)).xyz + 0.5f;

    ivec3 cellCoord, cellCoordEnd, di;
    vec3 dt, deltatx;
    setupUniformGridTraversal(x1, x2, cellDim, ivec3(opacityParameters.dimensions), cellCoord,
                              cellCoordEnd, di, dt, deltatx);

    bool continueTraversal = true;

    vec3 minMax = getNormalizedVoxel(opacity, opacityParameters, cellCoord).xyz;

    continueTraversal = stepToNextCellNoHit(deltatx, di, cellCoordEnd, dt, cellCoord);

    while(continueTraversal) {
        vec3 gridMinMaxVal = getNormalizedVoxel(opacity, opacityParameters, cellCoord).xyz;
        minMax = vec3(min(minMax.x, gridMinMaxVal.x), max(minMax.y, gridMinMaxVal.y), gridMinMaxVal.z);

        continueTraversal = stepToNextCellNoHit(deltatx, di, cellCoordEnd, dt, cellCoord);
    }

    return minMax.xy;
}