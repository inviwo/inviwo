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

vec3 hue2rgb(float hue) {
    float r = abs(hue * 6.0 - 3.0) - 1.0;
    float g = 2.0 - abs(hue * 6.0 - 2.0);
    float b = 2.0 - abs(hue * 6.0 - 4.0);
    return clamp(vec3(r, g, b), 0.0, 1.0);
}

vec3 rgb2hcv(vec3 rgb) {
    const float epsilon = 1e-10;

    vec4 p = (rgb.g < rgb.b) ? vec4(rgb.bg, -1.0, 2.0/3.0) : vec4(rgb.gb, 0.0, -1.0/3.0);
    vec4 q = (rgb.r < p.x) ? vec4(p.xyw, rgb.r) : vec4(rgb.r, p.yzx);
    float c = q.x - min(q.w, q.y);
    float h = abs((q.w - q.y) / (6.0 * c + epsilon) + q.z);

    return vec3(h, c, q.x);
}

vec3 hsv2rgb(vec3 hsv) {
    vec3 rgb = hue2rgb(hsv.x);

    return ((rgb - 1.0) * hsv.y + 1.0) * hsv.z;
}

vec3 hsl2rgb(vec3 hsl) {
    vec3 rgb = hue2rgb(hsl.x);
    float c = (1 - abs(2 * hsl.z - 1)) * hsl.y;

    return (rgb - 0.5) * c + hsl.z;
}

vec3 hcy2rgb(vec3 hcy) {
    const vec3 hcyWeights = vec3(0.299, 0.587, 0.114);

    vec3 rgb = hue2rgb(hcy.x);
    float z = dot(rgb, hcyWeights);

    if (hcy.z < z) {
        hcy.y *= hcy.z / z;
    }
    else if (z < 1.0) {
        hcy.y *= (1.0 - hcy.z) / (1.0 - z);
    }

    return (rgb - z) * hcy.y + hcy.z;
}
 
vec3 hcl2rgb(vec3 hcl) {
    const float PI = 3.1415926536;
    const float hclGamma = 3.0;
    const float hclY0 = 100.0;
    const float hclMaxL = 0.530454533953517;

    vec3 rgb = vec3(0.0);

    if (hcl.z != 0.0) {
        float h = hcl.x;
        float c = hcl.y;
        float l = hcl.z * hclMaxL;
        float q = exp((1.0 - c / (2.0 * l)) * (hclGamma / hclY0));
        float u = (2.0 * l - c) / (2.0 * q - 1.0);
        float v = c / q;
        float t = tan((h + min(fract(2.0 * h) / 4.0, fract(-2.0 * h) / 8.0)) * PI * 2.0);
        h *= 6.0;
        if (h <= 1.0) {
            rgb.r = 1.0;
            rgb.g = t / (1 + t);
        }
        else if (h <= 2.0) {
            rgb.r = (1.0 + t) / t;
            rgb.g = 1.0;
        }
        else if (h <= 3.0) {
            rgb.g = 1.0;
            rgb.b = 1.0 + t;
        }
        else if (h <= 4.0) {
            rgb.g = 1.0 / (1.0 + t);
            rgb.b = 1.0;
        }
        else if (h <= 5.0) {
            rgb.r = -1 / t;
            rgb.b = 1;
        }
        else {
            rgb.r = 1;
            rgb.b = -t;
        }
        rgb = rgb * v + u;
    }

    return rgb;
}

vec3 rgb2hsv(vec3 rgb) {
    const float epsilon = 1e-10;

    vec3 hcv = rgb2hcv(rgb);
    float s = hcv.y / (hcv.z + epsilon);

    return vec3(hcv.x, s, hcv.z);
}

vec3 rgb2hsl(vec3 rgb) {
    const float epsilon = 1e-10;

    vec3 hcv = rgb2hcv(rgb);
    float l = hcv.z - hcv.y * 0.5;
    float s = hcv.y / (1.0 - abs(l * 2.0 - 1.0) + epsilon);

    return vec3(hcv.x, s, l);
}

vec3 rgb2hcy(vec3 rgb) {
    const float epsilon = 1e-10;
    const vec3 hcyWeights = vec3(0.299, 0.587, 0.114);

    vec3 hcv = rgb2hcv(rgb);
    float y = dot(rgb, hcyWeights);
    float z = dot(hue2rgb(hcv.x), hcyWeights);

    if (y < z) {
      hcv.y *= z / (epsilon + y);
    }
    else {
      hcv.y *= (1.0 - z) / (epsilon + 1.0 - y);
    }

    return vec3(hcv.x, hcv.y, y);
}

vec3 rgb2hcl(vec3 rgb) {
    const float PI = 3.1415926536;
    const float hclGamma = 3.0;
    const float hclY0 = 100.0;
    const float hclMaxL = 0.530454533953517;

    vec3 hcl;
    float h = 0.0;
    float u = min(rgb.r, min(rgb.g, rgb.b));
    float v = max(rgb.r, max(rgb.g, rgb.b));
    float q = hclGamma / hclY0;
    hcl.y = v - u;

    if (hcl.y != 0.0) {
      h = atan(rgb.g - rgb.b, rgb.r - rgb.g) / PI;
      q *= u / v;
    }

    q = exp(q);
    hcl.x = fract(h / 2.0 - min(fract(h), fract(-h)) / 6.0);
    hcl.y *= q;
    hcl.z = mix(-u, v, q) / (hclMaxL * 2.0);

    return hcl;
}
