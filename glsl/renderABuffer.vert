/**
 * Fast Single-pass A-Buffer using OpenGL 4.0
 * Copyright Cyril Crassin, June 2010
**/

#version 400

uniform mat4 projectionMat;
uniform mat4 modelViewMat;
uniform mat4 modelViewMatIT;

in vec3 vertexPos;
in vec3 vertexNormal;

smooth out vec4 fragPos;
smooth out vec3 fragTexCoord;

smooth out vec3 fragNormal;


void main(){
	vec4 pos=projectionMat * modelViewMat*vec4(vertexPos.xyz, 1.0f);

	vec3 normalEye=normalize( (modelViewMatIT*vec4(vertexNormal, 1.0f)).xyz );

	fragTexCoord.xy=vertexPos.xy;
	fragTexCoord.z=abs(normalEye.z);
	
	fragNormal=normalEye;
	
	fragPos=pos;
	gl_Position = pos;
}