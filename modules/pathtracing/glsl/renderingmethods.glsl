#include "random.glsl"
#include "transmittancemethods.glsl"


struct plane {
    vec3 position;
    vec3 normal;
};

mat3 rotMatrixAroundAxel(vec3 v, float theta) {
    float costheta = cos(theta); float sintheta = sin(theta);
    mat3 thetaRot = mat3(                                                
        vec3(costheta+v.x*v.x*(1-costheta), 
            v.x*v.y*(1-costheta)-v.z*sintheta, 
                v.x*v.z*(1-costheta)+v.y*sintheta),
        vec3(v.y*v.x*(1-costheta)+v.z*sintheta, 
            costheta+v.y*v.y*(1-costheta), 
                v.y*v.z*(1-costheta)-v.x*sintheta),
        vec3(v.z*v.x*(1-costheta)-v.y*sintheta, 
            v.z*v.y*(1-costheta)+v.x*sintheta, 
                costheta+v.z*v.z*(1-costheta))
    );

    return thetaRot;
}

vec3 rotateFrom(vec3 v, float theta, float phi) {
    
    // produce for mat3's one for v, and one for v perpendicular
    mat3 thetaRot = rotMatrixAroundAxel(v, theta);

    // så länge v är inte ungefär lika med e_z (med epsilon marginal) så 'är det intuitivt' att det existerar
    // en punkt på planet som bildas av v som normal där z = 0, denna punkt kommer var ortogonal med v

    // is it this thats causing artifacts when looking at from z.
    // makes sense, but sketch it out on paper to make sure
    vec3 w = vec3(-v.y, v.x, 0f);
    if(v.x == 0 && v.y == 0) {
        w = vec3(0,0,1);
    }
    w = normalize(w);
    mat3 phiRot = rotMatrixAroundAxel(w, phi);
    //thetaRot*phiRot*v
    vec3 axis = thetaRot*phiRot*v;

    return axis;
}

//TODO: Use a struct{vec3 position, vec3 normal instead}

PlaneParameters[6] constructBBPlanes(VolumeParameters volParam) {
    PlaneParameters[6] planes;
    
    float dimx = volParam.dimensions.x;
    float dimy = volParam.dimensions.y;
    float dimz = volParam.dimensions.z;

    //normals in or out? in, according to rayPlaneIntersection
    planes[0] = PlaneParameters(vec3(dimx/2f, 0f, 0f), vec3(-1f, 0f, 0f), /*irrelevant*/ vec4(0f));
    planes[1] = PlaneParameters(vec3(-dimx/2f, 0f, 0f), vec3(1f, 0f, 0f), /*irrelevant*/ vec4(0f));
    planes[2] = PlaneParameters(vec3(0f, dimy/2f, 0f), vec3(0f, -1f, 0f), /*irrelevant*/ vec4(0f));
    planes[3] = PlaneParameters(vec3(0f, -dimy/2f, 0f), vec3(0f, 1f, 0f), /*irrelevant*/ vec4(0f));
    planes[4] = PlaneParameters(vec3(0f, 0f, dimz/2f), vec3(0f, 0f, -1f), /*irrelevant*/ vec4(0f));
    planes[5] = PlaneParameters(vec3(0f, 0f, -dimz/2f), vec3(0f, 0f, 1f), /*irrelevant*/ vec4(0f));

    return planes;
}

// We can actually just check bounds based on volume dimensions. Box intersection should still work the same.

// Overengineered? Prolly.
bool RayBBIntersection(PlaneParameters[6] planes, vec3 rayOrigin, vec3 rayDir, out float t0) {

    int foo = -1;
    int i = 0;
    float earliestIntersection = 100; // FLT_MAX 3.402823466e+38

    float intersection = 0f;
    while(i < 6) {
        intersection = 0f;
        if(rayPlaneIntersection(planes[i], rayOrigin, rayDir, intersection, 100f)) {
            foo = i;
            if(earliestIntersection > intersection) {
                earliestIntersection = intersection;
            }
        }
        i++;
    }
    t0 = earliestIntersection;
    if(foo > -1) {
        return true;
    }
    return false;
}

