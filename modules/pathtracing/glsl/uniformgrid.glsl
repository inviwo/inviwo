

#define OPTIMIZE_STEP_FOR_SIMD

// mix(x, y, a) = x*(1-a)+y*a
// select(x, y, a) = a ? y : x
vec3 select(vec3 x, vec3 y, bvec3 a) { return mix(x, y, a); }

void setupUniformGridTraversal(vec3 x1, vec3 x2, vec3 cellDim, ivec3 maxCells, out ivec3 cellCoord,
                               out ivec3 cellCoordEnd, out ivec3 di, out vec3 dt,
                               out vec3 deltatx) {
    vec3 cellCoordf = clamp(vec3(floor((x1) / cellDim)), vec3(0.f), vec3(maxCells - 1));

    cellCoord = ivec3(cellCoordf);

    cellCoordEnd = clamp(ivec3(x2 / cellDim), ivec3(0), maxCells - 1);

    bvec3 x1LessThanx2 = lessThan(x1, x2);
    bvec3 x1GreaterThanx2 = greaterThan(x1, x2);

    di = mix(mix(ivec3(0), ivec3(-1), x1GreaterThanx2), ivec3(1), x1LessThanx2);

    // Determine dt, the values of t at wich the directed segment
    // x1-x2 crosses the first horizontal and vertical cell
    // boundaries, respectively. Min(dt) indicates how far
    // we can travel along the segment and still remain in
    // the current cell
    vec3 invAbsDir = 1.f / abs(x2 - x1);

    // cellDim * cellCoordf
    vec3 minx = cellDim * cellCoordf;
    vec3 maxx = minx + cellDim;
    dt = mix(maxx - x1, x1 - minx, x1GreaterThanx2) * invAbsDir;

    // Determine delta x, how far in units for t we must
    // step along the directed line segment for the
    // horizontal/vertical/depth movement to equal
    // the width/height/depth of a cell
    // t is parameterized as [0 1] along the ray.

    deltatx = cellDim * invAbsDir;
}

bool stepToNextCellHit(vec3 deltatx, ivec3 di, ivec3 cellCoordEnd, inout vec3 dt,
                       inout ivec3 cellCoord, out float tHit) {
    bvec3 advance = bvec3((dt.x <= dt.y && dt.x <= dt.z), (dt.x > dt.y && dt.y <= dt.z),
                          (dt.x > dt.z && dt.y > dt.z));

    tHit = advance.x != false ? dt.x : (advance.y != false ? dt.y : dt.z);

    bvec3 cellCoordsEqual = equal(cellCoord, cellCoordEnd);
    if (any(mix(bvec3(false), advance, cellCoordsEqual))) {
        return false;
    }


    dt += mix(vec3(0), deltatx, advance);
    cellCoord += ivec3(mix(ivec3(0), di, advance));


    return true;
}

bool stepToNextCellNoHit(vec3 deltatx, ivec3 di, ivec3 cellCoordEnd, inout vec3 dt,
                         inout ivec3 cellCoord) {
    //#ifdef OPTIMIZE_STEP_FOR_SIMD
    bvec3 advance = bvec3((dt.x <= dt.y && dt.x <= dt.z), (dt.x > dt.y && dt.y <= dt.z),
                          (dt.x > dt.z && dt.y > dt.z));

    bvec3 cellCoordsEqual = equal(cellCoord, cellCoordEnd);
    // corresponds to bvec3 && bvec3
    // bvec3(advance.x && cellCoordsEqual.x, advance.y && cellCoordsEqual.y,
    //      advance.z && cellCoordsEqual.z);
    if (any(mix(bvec3(false), advance, cellCoordsEqual))) {
        return false;
    }

    dt += mix(vec3(0), deltatx, advance);
    cellCoord += ivec3(mix(ivec3(0), di, advance));

    //#else

    //#endif

    return true;
}