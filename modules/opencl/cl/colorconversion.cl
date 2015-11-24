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

#include "transformations.cl"

/**
* \brief Convert from HSV to RGB color.
*
* See http://en.wikipedia.org/wiki/HSL_and_HSV and
* http://en.wikipedia.org/wiki/RGB_color_model
* for a detailed explanation of the color spaces.
*
* @param hsv Color in the [0 1]^3 range.
* @return RGB color in [0 1]^3 range.
*/
float3 hsv2rgb(float3 hsv) {
    float hue = hsv.x;
    float sat = hsv.y;
    float val = hsv.z;
    float r = 0.f, g = 0.f, b = 0.f;
    if (sat < 1.e-8) { // only value, no saturation
        r = val;
        g = val;
        b = val;
        return (float3)(r, g, b);;
    }

    hue *= 360.f;

    // divide hue into six segments, 60 degree each
    int h_i = convert_int(floor(hue / 60.f)) % 6;
    float f = hue / 60.f - floor(hue / 60.f);
    float p = val * (1.f - sat);
    float q = val * (1.f - f * sat);
    float t = val * (1.f - (1.f - f) * sat);

    switch (h_i) {
    case 0:
        r = val;
        g = t;
        b = p;
        break;
    case 1:
        r = q;
        g = val;
        b = p;
        break;
    case 2:
        r = p;
        g = val;
        b = t;
        break;
    case 3:
        r = p;
        g = q;
        b = val;
        break;
    case 4:
        r = t;
        g = p;
        b = val;
        break;
    case 5:
        r = val;
        g = p;
        b = q;
        break;
    }
    return (float3)(r, g, b);
}

/**
* \brief Convert from RGB to HSV color.
*
* See http://en.wikipedia.org/wiki/HSL_and_HSV and
* http://en.wikipedia.org/wiki/RGB_color_model
* for a detailed explanation of the color spaces.
*
* @param rgb Color in the [0 1]^3 range.
* @return HSV color in the [0 1]^3 range.
*/
float3 rgb2hsv(float3 rgb) {
    float val = max(max(rgb.x, rgb.y), rgb.z);
    float sat = min(min(rgb.x, rgb.y), rgb.z);
    float range = val - sat;
    // set hue to zero for undefined values
    bool notGray = (fabs(range) > 1.e-8);
    float hue = 0.f;
    // blue hue
    if (notGray) {
        if (rgb.z == val)
            hue = 2.f / 3.f + (1.f / 6.f) * (rgb.x - rgb.y) / range;
        else if (rgb.y == val)
            hue = 1.f / 3.f + (1.f / 6.f) * (rgb.z - rgb.x) / range;
        else if (rgb.x == val)
            hue = (1.f / 6.f) * (rgb.y - rgb.z) / range;
    }

    if (hue < 0.0) {
        hue += 1.0;
    }
    if (notGray) {
        sat = 1.0 - sat / val;
    } else {
        sat = 0.0;
    }
    return (float3)(hue, sat, val);
}

float3 xyz2lab(float3 xyz, float3 whitePoint /*= float3(0.95047f, 1.f, 1.08883f)*/) {
    const float epsilon = 0.008856f; 
    const float kappa = 903.3f;
    float3 lab;
    float3 t = xyz/whitePoint;
    float3 f;
    f.x = t.x > epsilon ? pow(t.x, 1.f/3.f) : (kappa*t.x + 16.f)/116.f;  
    f.x = t.y > epsilon ? pow(t.y, 1.f/3.f) : (kappa*t.y + 16.f)/116.f;  
    f.z = t.z > epsilon ? pow(t.z, 1.f/3.f) : (kappa*t.z + 16.f)/116.f;  
    //for (int i = 0; i < 3; ++i) {
    //    if (t[i] > epsilon) {
    //        f[i] = pow(t[i], 1.f/3.f);
    //    } else {
    //        f[i] = (kappa*t[i] + 16.f)/116.f; 
    //    } 
    //}
    lab.x = 116.f * f.y - 16.f;
    lab.y = 500.f * (f.x - f.y);
    lab.z = 200.f * (f.y - f.z);
    return lab;
}

float3 lab2xyz(float3 lab, float3 whitePoint /*= float3(0.95047f, 1.f, 1.08883f)*/) {

    float3 t = (float3)( (1.f/116.f) * (lab.x + 16.f) );
    t.y += (1.f/500.f)*lab.y;
    t.z -= (1.f/200.f)*lab.z;
    float3 f;
    const float sixDivTwentyNine = 6.f/29.f;
    f.x = t.x > sixDivTwentyNine ? pow(t.x, 3.f) : 3.f*sixDivTwentyNine*sixDivTwentyNine*(t.x - 4.f/29.f); 
    f.y = t.y > sixDivTwentyNine ? pow(t.y, 3.f) : 3.f*sixDivTwentyNine*sixDivTwentyNine*(t.y - 4.f/29.f); 
    f.z = t.z > sixDivTwentyNine ? pow(t.z, 3.f) : 3.f*sixDivTwentyNine*sixDivTwentyNine*(t.z - 4.f/29.f); 

    //for (int i = 0; i < 3; ++i) {
    //    const float sixDivTwentyNine = 6.f/29.f;
    //    if (t[i] > sixDivTwentyNine) {
    //        f[i] = pow(t[i], 3.f);
    //    } else {
    //        // f(t) = 3*(6/29)^2 * (t - 4/29)            
    //        f[i] = 3.f*sixDivTwentyNine*sixDivTwentyNine*(t[i] - 4.f/29.f); 
    //    }
    //}
    float3 xyz;
    xyz.x = whitePoint.x * f.y;
    xyz.y = whitePoint.y * f.x;
    xyz.z = whitePoint.z * f.z;
    return xyz;
}

float3 rgb2xyz(float3 rgb) {
    // Conversion matrix for sRGB, D65 white point
    float16 rgb2xyzD65Mat = (float16)( 0.4124564f, 0.2126729f, 0.0193339f, 0.f,
                                  0.3575761f, 0.7151522f, 0.1191920f,0.f,
                                  0.1804375f, 0.0721750f, 0.9503041f, 0.f, (float4)(0.f));
    // Inverse sRGB companding
    float3 v;
    v.x = rgb.x > 0.04045f ? pow( (rgb.x + 0.055f) / 1.055f, 2.4f) : rgb.x / 12.92f;
    v.y = rgb.y > 0.04045f ? pow( (rgb.y + 0.055f) / 1.055f, 2.4f) : rgb.y / 12.92f;
    v.z = rgb.z > 0.04045f ? pow( (rgb.z + 0.055f) / 1.055f, 2.4f) : rgb.z / 12.92f;
    //for (int i = 0; i < 3; ++i) {
    //    if (rgb[i] > 0.04045f) {
    //        v[i] = pow( (rgb[i] + 0.055f) / 1.055f, 2.4f);
    //    } else {
    //        v[i] = rgb[i] / 12.92f;
    //    }
    //}
    return transformVector(rgb2xyzD65Mat, v);
}

float3 xyz2rgb(float3 xyz) {
    // Conversion matrix for sRGB, D65 white point
    float16 xyz2rgbD65Mat = (float16)( 3.2404542f, -0.9692660f,  0.0556434, 0.f,
                                    -1.5371385f,  1.8760108f, -0.2040259f, 0.f,
                                    -0.4985314f,  0.0415560f,  1.0572252f, 0.f, (float4)(0.f));
    float3 v = transformVector(xyz2rgbD65Mat, xyz);
    // sRGB companding
    float3 rgb;
    rgb.x = v.x > 0.0031308f ? pow(v.x, 1.f/2.4f) * 1.055f - 0.055f : v.x * 12.92f;
    rgb.y = v.y > 0.0031308f ? pow(v.y, 1.f/2.4f) * 1.055f - 0.055f : v.y * 12.92f;
    rgb.z = v.z > 0.0031308f ? pow(v.z, 1.f/2.4f) * 1.055f - 0.055f : v.z * 12.92f;
    //for (int i = 0; i < 3; ++i) {
    //    if (v[i] > 0.0031308f) {
    //        rgb[i] = pow(v[i], 1.f/2.4f) * 1.055f - 0.055f;
    //    } else {
    //        rgb[i] = v[i] * 12.92f;
    //    }
    //}
    return rgb;

}

float3 rgb2lab(float3 rgb) {
    float3 xyz = rgb2xyz(rgb);

    return xyz2lab(xyz, (float3)(0.95047f, 1.f, 1.08883f));
}

float3 lab2rgb(float3 lab) {
    float3 xyz = lab2xyz(lab, (float3)(0.95047f, 1.f, 1.08883f));

    return xyz2rgb(xyz);
}