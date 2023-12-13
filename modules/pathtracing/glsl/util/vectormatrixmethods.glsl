#include "random.glsl"

/**
 * Test intersection between ray and axis-aligned bounding box.
 * If intersecting, t0 will contain the first point of intersection along the ray and t1 the last.
 * If ray is inside the box, t0 will be set to 0.
 * If t0 is negative it may contain the first hit point on the line (along the negative direction).
 *
 * @param bbMin Minimum point of axis-aligned bounding box
 * @param bbMax Maximum point of axis-aligned bounding box
 * @param rayOrigin   Ray origin
 * @param rayDir    Ray direction (not normalized)
 * @param t0   Start point along ray
 * @param t1   End point along ray
 * @return true if an intersection is found, false otherwise.
 */
bool rayBoxIntersection(vec3 bbMin, vec3 bbMax vec3 rayOrigin, vec3 rayDir,
                                     out float t0, out float t1) {
    float FLT_MAX = 3.402823466e+38;

    vec3 invDir = 1.f / dir;
    vec3 tNearV = (bbMin - origin) * invDir;
    vec3 tFarV = (bbMax - origin) * invDir;
    vec3 tNear = min(tNearV, tFarV);
    vec3 tFar = max(tNearV, tFarV);

    t0 = max(t0, max(max(tNear.x, tNear.y), tNear.z));
    t1 = min(t1, min(min(tFar.x, tFar.y), tFar.z));
    return (t0 < t1) && (t1 < FLT_MAX);
}