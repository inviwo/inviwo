/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

uniform sampler2D vectorFieldColor;
uniform sampler2D noiseTextureColor;

uniform int samples;
uniform float stepLength;
uniform bool normalizeVectors;
uniform bool intensityMapping;
uniform bool useRK4;

in vec3 texCoord_;


vec2 euler(vec2 posF ){
    vec2 V0 = texture(vectorFieldColor, posF).rg;
    if(normalizeVectors){
        V0 = normalize(V0);
    }
    return V0;

}
 
vec2 rk4(vec2 p0 , float stepsize) {
    vec2 V0 = euler(p0);
    
    vec2 p1 = p0 + V0 * stepsize/2;
    vec2 V1 = euler(p1);
    
    vec2 p2 = p0 + V1 * stepsize/2;
    vec2 V2 = euler(p2);

    vec2 p3 = p0 + V2 * stepsize;
    vec2 V3 = euler(p3);


    return (V0 + 2*(V1+V2) + V3) / 6.0;
}

void traverse(inout  float v , inout int c,vec2 posF , float stepSize,int steps){
    for(int i = 0;i<steps;i++){
        vec2 V0;
        if(useRK4){
            V0 = rk4(posF,stepSize);
        }else{
            V0 = euler(posF);
        }
        posF += V0 * stepSize;

        if(posF.x < 0 ) break;
        if(posF.y < 0 ) break;

        if(posF.x > 1 ) break;
        if(posF.y > 1 ) break;

        v += texture(noiseTextureColor, posF.xy).r;
        c += 1;
    }
}




void main() {
	float v = texture(noiseTextureColor, texCoord_.xy).r;

	int c = 1;
	traverse(v,c,texCoord_.xy , stepLength , samples / 2);
    traverse(v,c,texCoord_.xy , -stepLength , samples / 2);

	v /= c;

    if(intensityMapping)
        v = pow(v,(5.0/pow((v+1.0),4)));

    FragData0 = vec4(v,v,v,1.0);
}