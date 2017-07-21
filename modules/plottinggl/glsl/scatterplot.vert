/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

layout(location = 0) in float X;
layout(location = 1) in float Y;
layout(location = 2) in float C;
layout(location = 3) in float R;


#include "plotting/common.glsl"
#include "utils/structs.glsl"
#include "utils/sampler2d.glsl"

uniform sampler2D transferFunction;

out vec4 vColor;
out float vRadius;
out float vDepth;

uniform vec2 minmaxX;
uniform vec2 minmaxY;
uniform vec2 minmaxC;
uniform vec2 minmaxR;
uniform vec4 default_color;


uniform float minRadius;
uniform float maxRadius;

uniform int has_color;
uniform int has_radius;


float norm(in float v, in vec2 mm ){
    return (v - mm.x) /(mm.y - mm.x);

}

void main(void) {
    float x = norm(X,minmaxX);
    float y = norm(Y,minmaxY);

    vec2 pixel = getPixelCoordsWithSpacing(vec2(x,y));


    float c = 1;

    if(has_color==1){
        c = norm(C,minmaxC);
        vColor = texture(transferFunction , vec2(c,0.5));
    }else{
        vColor = default_color;
    }

    if(has_radius==1){
        float r = norm(R,minmaxR);
        vDepth = r;
        vRadius = minRadius + r * (maxRadius-minRadius);    
    }else{
        vRadius = maxRadius;
        vDepth = 0.5;
    }

     

    gl_Position = vec4(pixel,0,0);
}
 