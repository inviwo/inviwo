


vec4 HelloWorld(vec4 inVec4) {


    vec4 outVec4 = vec4(1 - inVec4.x, 1 - inVec4.y, 1- inVec4.z, inVec4.w);
    
    float xDiff = abs(outVec4.x - inVec4.x);
    float yDiff = abs(outVec4.y - inVec4.y);
    float zDiff = abs(outVec4.z - inVec4.z);

    outVec4.x = outVec4.x/xDiff;
    outVec4.y = outVec4.y/yDiff;
    outVec4.z = outVec4.z/zDiff;
    return inVec4;
}

vec4 RMVolumeRender_simple(inout float T, float rayStep, float tau, vec4 tfSample, vec4 acc_radiance) {
    acc_radiance = acc_radiance + T*tau*tfSample*rayStep;
    T = T*exp(-tau*rayStep);

    return acc_radiance;
}

vec4 RMVolumeRender_simpleWithLight(inout float T, float rayStep, float tau, vec4 tfSample, vec4 acc_radiance, 
    vec3 samplePos, vec3 lightPos, float lightRadiance) {
    
    float lightCoeff = lightRadiance/pow(distance(vec3(4,4,0), samplePos), 2f);
    vec4 lightColor = vec4(1.0f)*lightCoeff;
    // what am i doing wrong? Why am i incapable of getting a gradient from the light?
    // Because im normalizing... dont do any scaling off of a vector
    //lightColor = vec4((vec3(2) - samplePos), 1f);
    //lightColor = vec4(length(lightColor)*0.05);
    //lightColor *= 0.3f;
    //lightColor.y -= -3;
    
    lightColor.w = 1f;


    acc_radiance = acc_radiance + T*tau*(tfSample + lightColor)*rayStep;
    T = T*exp(-tau*rayStep);

    return acc_radiance;
}