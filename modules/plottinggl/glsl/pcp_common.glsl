uniform ivec2 dims;

//    vec4(top,right,bottom,left)
uniform vec4 spacing;
                                            
vec2 getPosWithSpacing(vec2 inPos){
    //return inPos*2.0-1.0;
    vec2 pixelSpace = (inPos * (dims - spacing.wz - spacing.yx) + spacing.wz);
    return pixelSpace / (dims/2.0) - 1.0;
} 


vec2 getPixelSpacing() {
    return 1.0f / dims.xy;
}

