#include "pcp_common.glsl"

uniform float x;

void main(){
    gl_Position = vec4(getPosWithSpacing(vec2(x,in_Vertex.y)),0,1);
} 