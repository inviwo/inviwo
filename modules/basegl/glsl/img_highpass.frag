/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 *********************************************************************************/

#include "utils/structs.glsl"

uniform ImageParameters outportParameters_;

uniform sampler2D inport_;

uniform int kernelSize;
uniform bool sharpen;

void main() {
	vec4 p = texture(inport_,gl_FragCoord.xy* outportParameters_.reciprocalDimensions);
	vec3 v = vec3(0);
	int k2 = kernelSize/2;
	int startX = int(gl_FragCoord.x) - k2;
	int endX = int(gl_FragCoord.x) + k2;
	int startY = int(gl_FragCoord.y) - k2;
	int endY = int(gl_FragCoord.y) + k2;
	int w = 0;
	for(int y = startY  ; y < endY ; y++){
		for(int x = startX  ; x < endX ; x++){
			if(x == gl_FragCoord.x && y == gl_FragCoord.y) continue;
			v += texture(inport_,vec2(x,y) * outportParameters_.reciprocalDimensions).xyz;
			w++;
		}
	}

	vec3 outV;
	if(sharpen){
		outV = 2*p.xyz - v/w;
	}else{
		outV = p.xyz - v/w;
		outV += 1;
		outV /= 2;
	}

    FragData0 = vec4( outV , p.a) ;
}
