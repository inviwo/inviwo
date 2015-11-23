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

#include <algorithm>

namespace inviwo {


IVW_CORE_API vec3 hsv2rgb(vec3 hsv) {
    double hue = hsv.x;
    double sat = hsv.y;
    double val = hsv.z;
    double r, g, b;

    if (sat < 1.0e-8) { // only value, no saturation
        r = val;
        g = val;
        b = val;
        return vec3(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b));;
    }

    hue *= 360.0;
    // divide hue into six segments, 60 degree each
    int h_i = static_cast<int>(std::floor(hue / 60.0)) % 6;
    double f = hue / 60.0 - std::floor(hue / 60.0);
    double p = val * (1.0 - sat);
    double q = val * (1.0 - f * sat);
    double t = val * (1.0 - (1.0 - f) * sat);

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
    return vec3(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b));
}

IVW_CORE_API vec3 rgb2hsv(vec3 rgb) {
    double r = rgb.x;
    double g = rgb.y;
    double b = rgb.z;
    double val = std::max(std::max(r, g), b);
    double sat = std::min(std::min(r, g), b);
    double range = val - sat;

    // set hue to zero for undefined values
    bool notGray = (std::abs(val - sat) > 1.0e-8);
    double hue = 0.0;

    // blue hue
    if (notGray) {
        if (b == val)
            hue = 2.0/3.0 + 1.0/6.0 * (r - g) / range;
        else if (g == val)
            hue = 1.0/3.0 + 1.0/6.0 * (b - r) / range;
        else if (r == val)
            hue = 1.0/6.0 * (g - b) / range;
    }

    if (hue < 0.0) {
        hue += 1.0;
    }
    if (notGray) {
        sat = 1.0 - sat / val;
    }
    else {
        sat = 0.0;
    }
    return vec3(static_cast<float>(hue), static_cast<float>(sat), static_cast<float>(val));
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

IVW_CORE_API vec3 rgb2ycbcr(const vec3 &rgb) {
    double r = rgb.x;
    double g = rgb.y;
    double b = rgb.z;

    double y = 0.299 * r + 0.587 * g + 0.114 * b;
    double cb = (b - y) * 0.565;
    double cr = (r - y) * 0.713;

    return vec3(static_cast<float>(y), static_cast<float>(cb), static_cast<float>(cr));
}

IVW_CORE_API vec3 ycbcr2rgb(const vec3 &ycbcr) {
    double y = ycbcr.x;
    double cb = ycbcr.y;
    double cr = ycbcr.z;

    double r = glm::clamp(y + 1.402 * cr, 0.0, 1.0);
    double g = glm::clamp(y - 0.344136 * cb - 0.714136 * cr, 0.0, 1.0);
    double b = glm::clamp(y + 1.772 * cb, 0.0, 1.0);

    return vec3(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b));
}

} // namespace
