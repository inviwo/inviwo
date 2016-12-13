/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2016 Inviwo Foundation
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

/*
 * Max, Min Functions.
 */
float maxCom(vec4 col) {
    return max(col.r, max(col.g,col.b));
}

float minCom(vec4 col) {
    return min(col.r, min(col.g,col.b));
}

 /*
 * Returns a vec4 with components h,s,l,a.
 */
vec4 rgbToHsl(vec4 col) {
    float maxComponent = maxCom(col);
    float minComponent = minCom(col);
    float dif = maxComponent - minComponent;
    float add = maxComponent + minComponent;
    vec4 outColor = vec4(0.0, 0.0, 0.0, col.a);
    
    if (minComponent == maxComponent) {
        outColor.r = 0.0;
    }
    else if (col.r == maxComponent) {
        outColor.r = mod(((60.0 * (col.g - col.b) / dif) + 360.0), 360.0);
    }
    else if (col.g == maxComponent) {
        outColor.r = (60.0 * (col.b - col.r) / dif) + 120.0;
    }
    else {
        outColor.r = (60.0 * (col.r - col.g) / dif) + 240.0;
    }

    outColor.b = 0.5 * add;
    
    if (outColor.b == 0.0) {
        outColor.g = 0.0;
    }
    else if (outColor.b <= 0.5) {
        outColor.g = dif / add;
    }
    else {
        outColor.g = dif / (2.0 - add);
    }
    
    outColor.r /= 360.0;
    
    return outColor;
}

/*
 * Returns a component based on luminocity p, saturation q, and hue h. 
 */
float hueToRgb(float p, float q, float h) {
    if (h < 0.0) {
        h += 1.0;
    }
    else if (h > 1.0) {
        h -= 1.0;
    }
    if ((h * 6.0) < 1.0) {
        return p + (q - p) * h * 6.0;
    }
    else if ((h * 2.0) < 1.0) {
        return q;
    }
    else if ((h * 3.0) < 2.0) {
        return p + (q - p) * ((2.0 / 3.0) - h) * 6.0;
    }
    else {
        return p;
    }
}
/*
 * Returns a vec4 with components r,g,b,a, based off vec4 col with components h,s,l,a.
 */
vec4 hslToRgb(vec4 col) {
    vec4 outColor = vec4(0.0, 0.0, 0.0, col.a);
    float p, q, tr, tg, tb;
    if (col.b <= 0.5) {
        q = col.b * (1.0 + col.g);
    }
    else {
        q = col.b + col.g - (col.b * col.g);
    }

    p = 2.0 * col.b - q;
    tr = col.r + (1.0 / 3.0);
    tg = col.r;
    tb = col.r - (1.0 / 3.0);

    outColor.r = hueToRgb(p, q, tr);
    outColor.g = hueToRgb(p, q, tg);
    outColor.b = hueToRgb(p, q, tb);

    return outColor;
}
