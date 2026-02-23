
struct Orbital
{

    vec4 p;
    ivec4 coefs;
    float alpha,c_Coeff,N_ijk,m_Coeff;

};

layout(std430, binding = 0) buffer GaussianBuffer {
    Orbital data[]; // points is an array of GaussianOrbital
};


vec3 padd(vec3 samplePos,vec3 minPad, vec3 maxPad)
{
    return minPad + samplePos*(maxPad-minPad);
}

float getNormalisedDensity(float density,float minValue, float maxValue)
{
    return (-minValue + density)/(maxValue-minValue);
}

vec4 calcDensity(vec3 samplePos, uint numPoints,vec3 bmin, vec3 bmax){
    vec4 density = vec4(0.0);
    
    
    /*vec3 c = 0.5f*(bmax + bmin);
    vec3 e = bmax - bmin;
    float L = max(e.x,max(e.y,e.z));
    bmin = c - 0.5f*vec3(L);
    bmax = c + 0.5f*vec3(L);*/
    vec3 posFloat = padd(samplePos,bmin,bmax);

    for(uint i = 0; i < numPoints; ++i) {
        
        /*float s = sigma;
        Orbital orb = data[i]; 
        vec3 dr = orb.p.xyz - samplePos;
        
        float r2 = dot(dr,dr);
        float A = 1.0 / (s*sqrt(2*M_PI));
        float B = .5/(s*s);
        float test = pow(abs(dr.x),orb.coefs.x)*pow(abs(dr.y),orb.coefs.y)*pow(abs(dr.z),orb.coefs.z)*4000;
        float v = test*exp(-B*r2);
        vec3 grad = -2*B*dr*v;*/
        Orbital pgto = data[i];
        vec3 dr = posFloat - pgto.p.xyz;
        float fx = pow(abs(dr.x),pgto.coefs.x);
        float fy = pow(abs(dr.y),pgto.coefs.y);
        float fz = pow(abs(dr.z),pgto.coefs.z);
        float expVal = exp(-pgto.alpha*dot(dr,dr));
        

        float v = pgto.m_Coeff * pgto.c_Coeff * pgto.N_ijk * fx * fy * fz * expVal;
        
        
        //float L = max(s.x,max(s.y,s.z));
        
        //density += val;
        vec3 grad = -2*pgto.alpha*dr*v;

        density += vec4(grad, v);
        


    }
    //result = vec4(0,1,1,1);
    
    return density*density*4;
}

