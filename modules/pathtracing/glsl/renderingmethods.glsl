#include "random.glsl"
#include "transmittancemethods.glsl"

mat3 rotMatrixAroundAxel(vec3 v, float theta) {
    float costheta = cos(theta); float sintheta = sin(theta);
    mat3 thetaRot = mat3(                                                
        vec3(costheta+v.x*v.x*(1-costheta), v.x*v.y*(1-costheta)-v.z*sintheta, v.x*v.z*(1-costheta)+v.y*sintheta),
        vec3(v.y*v.x*(1-costheta)+v.z*sintheta, costheta+v.y*v.y*(1-costheta), v.y*v.z*(1-costheta)-v.x*sintheta),
        vec3(v.z*v.x*(1-costheta)+v.z*sintheta, v.z*v.y*(1-costheta)+v.x*sintheta, costheta+v.z*v.z*(1-costheta))
    );

    return thetaRot;
}

vec3 rotateFrom(vec3 v, float theta, float phi) {
    
    // produce for mat3's one for v, and one for v perpendicular
    mat3 thetaRot = rotMatrixAroundAxel(v, theta);
    vec3 w = vec3(0,1f,0);
    if(v == vec3(0,1f,0)) {
        w = vec3(0,0,1f);
    }
    w = normalize(cross(v,w));
    mat3 phiRot = rotMatrixAroundAxel(w, phi);

    vec3 axis = thetaRot*phiRot*v;

    return axis;
}

