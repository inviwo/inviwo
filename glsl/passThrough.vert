/**
 * Fast Single-pass A-Buffer using OpenGL 4.0
 * Copyright Cyril Crassin, June 2010
**/

#version 400


in vec4 vertexPos;

smooth out vec4 fragPos;

void main(){
	fragPos=vertexPos;
	gl_Position = vertexPos;
}