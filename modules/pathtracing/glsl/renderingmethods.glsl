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


// We can actually just check bounds based on volume dimensions. Box intersection should still work the same.

// Assumes sample space to be from 000 to 111
bool RayBBIntersection_TextureSpace(vec3 origin, vec3 dir, out float t0, out float t1) {
    float FLT_MAX = 3.402823466e+38;
    
    vec3 invDir = 1.f/dir;
    // completely breaks if the bounding box points arent as close as 000 and 111 as a floating point can represent
    vec3 tNearV = (vec3(0f, 0f, 0f)- origin)*invDir;
    vec3 tFarV = (vec3(1f, 1f, 1f) - origin)*invDir;
    vec3 tNear = min(tNearV, tFarV);
    vec3 tFar = max(tNearV, tFarV);

    t0 = max(t0, max( max(tNear.x, tNear.y), tNear.z ) );
    t1 = min(t1, min( min(tFar.x, tFar.y), tFar.z ) );
    return (t0 < t1) && (t1 < FLT_MAX);
}