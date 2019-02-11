/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

in vec3 texCoord_;

uniform sampler2D tex;

uniform vec2 reciprocalDimensions;
uniform vec2 offset;
uniform float kernelScale;

uniform float kernel[KERNELSIZE];

void main() {
	vec2 tCord = gl_FragCoord.xy * reciprocalDimensions;

#if KERNELSIZE == 1
    FragData0 = texture(tex,tCord.xy); 
#else
	vec3 v = vec3(0); 
	
	float startX = gl_FragCoord.x - (KERNELWIDTH-1)/2.0;
	float startY = gl_FragCoord.y - (KERNELHEIGHT-1)/2.0;

	for(int j = 0  ; j < KERNELHEIGHT ; j++){
		for(int i = 0  ; i < KERNELWIDTH ; i++){
			float x = startX + i;
			float y = startY + j;
			vec2 p = vec2(x,y) * reciprocalDimensions;
			
			float w = kernel[i+j*KERNELWIDTH];
			v += texture(tex, p ).rgb * w;
		}
	}
	v /= kernelScale;
	
    FragData0 = vec4(v,texture(tex,tCord.xy).a); 
#endif
}
