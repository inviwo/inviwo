


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