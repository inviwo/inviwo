/**
 * Fast Single-pass A-Buffer using OpenGL 4.0
 * Copyright Cyril Crassin, June 2010
**/

#include "utils/structs.glsl"

smooth out vec4 fragPos;

void main(){
	fragPos=vec4(in_Vertex.xy, 0.0, 1.0);
	gl_Position = fragPos;
}