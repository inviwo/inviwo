/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

uniform ivec3 volumeSize_;

in vec4 texCoord_;
in vec4 dataposition_;

uniform vec2 xRange;
uniform vec2 yRange;
uniform vec2 zRange;

uniform float t;

float getPos(float v ,vec2 range){ 
	return range.x + v * (range.y - range.x);
}

vec4 getPos(){
	return vec4(
		      getPos(dataposition_.x , xRange)
			, getPos(dataposition_.y , yRange)
			, getPos(dataposition_.z , zRange)
			, t
		);
}

void main() {
    vec4 pos = getPos();

    vec4 value = vec4(0,0,0,1);
    value.x = X_VALUE(pos.x,pos.y,pos.z,pos.w);
    value.y = Y_VALUE(pos.x,pos.y,pos.z,pos.w);
    value.z = Z_VALUE(pos.x,pos.y,pos.z,pos.w);

    FragData0 = value;
}
