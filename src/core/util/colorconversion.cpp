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

#include <inviwo/core/util/colorconversion.h>


namespace inviwo {


IVW_CORE_API vec3 hsv2rgb(vec3 hsv) {
    float hue = hsv.x;
    float sat = hsv.y;
    float val = hsv.z;
    float x = 0.f, y = 0.f, z = 0.f;

    if (hue == 1.f)
        hue = 0.f;
    else
        hue *= 6.f;

    int i = int(glm::floor(hue));
    float f = hue-i;
    float p = val*(1-sat);
    float q = val*(1-(sat*f));
    float t = val*(1-(sat*(1-f)));

    switch (i) {
    case 0:
        x = val;
        y = t;
        z = p;
        break;

    case 1:
        x = q;
        y = val;
        z = p;
        break;

    case 2:
        x = p;
        y = val;
        z = t;
        break;

    case 3:
        x = p;
        y = q;
        z = val;
        break;

    case 4:
        x = t;
        y = p;
        z = val;
        break;

    case 5:
        x = val;
        y = p;
        z = q;
        break;
    }

    return vec3(x,y,z);
}

IVW_CORE_API vec3 rgb2hsv(vec3 rgb) {
    const float& x = rgb.r;
    const float& y = rgb.g;
    const float& z = rgb.b;
    float maximum = (x > y) ? ((x > z) ? x : z) : ((y > z) ? y : z);
    float minimum = (x < y) ? ((x < z) ? x : z) : ((y < z) ? y : z);
    float range = maximum - minimum;
    float val    = maximum;
    float sat   = 0.f;
    float hue   = 0.f;

    if (maximum != 0.f)
        sat = range/maximum;

    if (sat > 1e-7f) {
        float h;

        if (x == maximum)
            h = (y - z) / range;
        else if (y == maximum)
            h = 2.f + (z - x) / range;
        else
            h = 4.f + (x - y) / range;

        hue = h/6.f;

        if (hue < 0.f)
            hue += 1.f;
    }

    return vec3(hue,sat,val);
}


IVW_CORE_API vec3 xyz2lab(vec3 xyz, vec3 whitePoint /*= vec3(0.95047f, 1.f, 1.08883f)*/) {
    static const float epsilon = 0.008856f; 
    static const float kappa = 903.3f;
    vec3 lab;
    vec3 t = xyz/whitePoint;
    vec3 f;
    for (int i = 0; i < 3; ++i) {
        if (t[i] > epsilon) {
            f[i] = std::pow(t[i], 1.f/3.f);
        } else {
            f[i] = (kappa*t[i] + 16.f)/116.f; 
        }

    }
    lab.x = 116.f * f.y - 16.f;
    lab.y = 500.f * (f.x - f.y);
    lab.z = 200.f * (f.y - f.z);
    return lab;
}

IVW_CORE_API vec3 lab2xyz(const vec3 lab, const vec3 whitePoint /*= vec3(0.95047f, 1.f, 1.08883f)*/) {

    vec3 t( (1.f/116.f) * (lab.x + 16.f) );
    t.y += (1.f/500.f)*lab.y;
    t.z -= (1.f/200.f)*lab.z;
    vec3 f;
    for (int i = 0; i < 3; ++i) {
        const float sixDivTwentyNine = 6.f/29.f;
        if (t[i] > sixDivTwentyNine) {
            f[i] = std::pow(t[i], 3.f);
        } else {
            // f(t) = 3*(6/29)^2 * (t - 4/29)            
            f[i] = 3.f*sixDivTwentyNine*sixDivTwentyNine*(t[i] - 4.f/29.f); 
        }
    }
    vec3 xyz;
    xyz.x = whitePoint.x * f.y;
    xyz.y = whitePoint.y * f.x;
    xyz.z = whitePoint.z * f.z;
    return xyz;
}

IVW_CORE_API vec3 rgb2xyz(const vec3 rgb) {
    // Conversion matrix for sRGB, D65 white point
    static const mat3 rgb2xyzD65Mat( 0.4124564f, 0.2126729f, 0.0193339f,
                                  0.3575761f, 0.7151522f, 0.1191920f,
                                  0.1804375f, 0.0721750f, 0.9503041f);
    // Inverse sRGB companding
    vec3 v;
    for (int i = 0; i < 3; ++i) {
        if (rgb[i] > 0.04045f) {
            v[i] = std::pow( (rgb[i] + 0.055f) / 1.055f, 2.4f);
        } else {
            v[i] = rgb[i] / 12.92f;
        }
    }
    return rgb2xyzD65Mat*v;
}

IVW_CORE_API vec3 xyz2rgb(const vec3 xyz) {
    // Conversion matrix for sRGB, D65 white point
    static const mat3 xyz2rgbD65Mat( 3.2404542f, -0.9692660f,  0.0556434,
                                    -1.5371385f,  1.8760108f, -0.2040259f,
                                    -0.4985314f,  0.0415560f,  1.0572252f);
    vec3 v = xyz2rgbD65Mat*xyz;
    // sRGB companding
    vec3 rgb;
    for (int i = 0; i < 3; ++i) {
        if (v[i] > 0.0031308f) {
            rgb[i] = std::pow(v[i], 1.f/2.4f) * 1.055f - 0.055f;
        } else {
            rgb[i] = v[i] * 12.92f;
        }
    }
    return rgb;

}

IVW_CORE_API vec3 rgb2lab(const vec3 rgb) {
    vec3 xyz = rgb2xyz(rgb);

    return xyz2lab(xyz);
}

IVW_CORE_API vec3 lab2rgb(const vec3 lab) {
    vec3 xyz = lab2xyz(lab);

    return xyz2rgb(xyz);
}

} // namespace
