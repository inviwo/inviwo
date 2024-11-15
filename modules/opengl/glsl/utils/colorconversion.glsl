/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#ifndef IVW_COLOR_CONVERSION_GLSL
#define IVW_COLOR_CONVERSION_GLSL

/**
 * \brief reference white point D65 in CIE XYZ color space
 *
 * The values are obtained by converting sRGB(1, 1, 1) to XYZ using rgb2XYZ() since sRGB assumes D65
 * as reference white.
 * See rgb2XYZ, http://www.brucelindbloom.com
 */
const vec3 whitePoint = vec3(0.95047, 1.0, 1.08883); 


float calcLuminance(vec3 rgbColor) {
    return 0.299 * rgbColor.r + 0.587 * rgbColor.g + 0.114 * rgbColor.b;
}

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

/**
 * \brief Convert from RGB to HSV color.
 *
 * See http://en.wikipedia.org/wiki/HSL_and_HSV and
 * http://en.wikipedia.org/wiki/RGB_color_model
 * for a detailed explanation of the color spaces.
 *
 * @param rgb Color in the [0 1]^3 range
 * @return HSV color in the [0 1]^3 range
 */
vec3 rgb2hsv(vec3 rgb) {
    const float epsilon = 1e-10;

    vec3 hcv = rgb2hcv(rgb);
    float s = hcv.y / (hcv.z + epsilon);

    return vec3(hcv.x, s, hcv.z);
}

/**
 * \brief Convert from HSV to RGB color.
 *
 * See http://en.wikipedia.org/wiki/HSL_and_HSV and
 * http://en.wikipedia.org/wiki/RGB_color_model
 * for a detailed explanation of the color spaces.
 *
 * @param hsv Color in the [0 1]^3 range
 * @return RGB color in [0 1]^3 range
 */
vec3 hsv2rgb(vec3 hsv) {
    vec3 rgb = hue2rgb(hsv.x);

    return ((rgb - 1.0) * hsv.y + 1.0) * hsv.z;
}

/**
 * \brief Convert from HSV to RGB color.
 *
 * See http://en.wikipedia.org/wiki/HSL_and_HSV and
 * http://en.wikipedia.org/wiki/RGB_color_model
 * for a detailed explanation of the color spaces.
 *
 * @param hsl Color in the [0 1]^3 range
 * @return RGB color in [0 1]^3 range
 */
vec3 hsl2rgb(vec3 hsl) {
    vec3 rgb = hue2rgb(hsl.x);
    float c = (1 - abs(2 * hsl.z - 1)) * hsl.y;

    return (rgb - 0.5) * c + hsl.z;
}

/**
 * \brief Convert from RGB to HSV color.
 *
 * See http://en.wikipedia.org/wiki/HSL_and_HSV and
 * http://en.wikipedia.org/wiki/RGB_color_model
 * for a detailed explanation of the color spaces.
 *
 * @param rgb Color in the [0 1]^3 range
 * @return HSL color in the [0 1]^3 range
 */
vec3 rgb2hsl(vec3 rgb) {
    const float epsilon = 1e-10;

    vec3 hcv = rgb2hcv(rgb);
    float l = hcv.z - hcv.y * 0.5;
    float s = (l > epsilon && l < 1.0 - epsilon) ? hcv.y / (1.0 - abs(l * 2.0 - 1.0)) : 0.0;

    return vec3(hcv.x, s, l);
}

/**
 * \brief Convert from XYZ to Lab color space
 * See http://en.wikipedia.org/wiki/CIE_1931_color_space
 * and http://www.brucelindbloom.com/
 * @param xyz color in the CIE XYZ color space (roughly in the [0 1]^3 range)
 * @param whitePointXYZ Normalized white point. Default white point is D65.
 * @return Lab color
 */
vec3 XYZ2lab(vec3 xyz, vec3 whitePointXYZ = whitePoint) {
    const float epsilon = 0.008856f;
    const float kappa = 903.3f;
    vec3 lab;
    vec3 t = xyz / whitePointXYZ;
    vec3 f;
    for (int i = 0; i < 3; ++i) {
        if (t[i] > epsilon) {
            f[i] = pow(t[i], 1.f / 3.f);
        } else {
            f[i] = (kappa * t[i] + 16.f) / 116.f;
        }
    }
    lab.x = 116.f * f.y - 16.f;
    lab.y = 500.f * (f.x - f.y);
    lab.z = 200.f * (f.y - f.z);
    return lab;
}

/**
 * \brief Convert from Lab to CIE XYZ color space
 * See http://en.wikipedia.org/wiki/Lab_color_space
 * and http://www.brucelindbloom.com/
 * @param lab Color
 * @param whitePoint Normalized white point. Default white point is D65.
 * @return CIE XYZ color (roughly in the [0 1]^3 range)
 */
vec3 lab2XYZ(vec3 lab, const vec3 whitePointXYZ = whitePoint) {
    vec3 t = vec3((1.f / 116.f) * (lab.x + 16.f));
    t.y += (1.f / 500.f) * lab.y;
    t.z -= (1.f / 200.f) * lab.z;
    vec3 f;
    for (int i = 0; i < 3; ++i) {
        const float sixDivTwentyNine = 6.f / 29.f;
        if (t[i] > sixDivTwentyNine) {
            f[i] = pow(t[i], 3.f);
        } else {
            // f(t) = 3*(6/29)^2 * (t - 4/29)
            f[i] = 3.f * sixDivTwentyNine * sixDivTwentyNine * (t[i] - 4.f / 29.f);
        }
    }
    vec3 xyz;
    xyz.x = whitePoint.x * f.y;
    xyz.y = whitePoint.y * f.x;
    xyz.z = whitePoint.z * f.z;
    return xyz;
}

/** 
 * \brief Convert from XYZ color to sRGB using D65 white point.
 *
 * See http://en.wikipedia.org/wiki/CIE_1931_color_space,
 * http://www.brucelindbloom.com/
 * and http://en.wikipedia.org/wiki/RGB_color_model 
 * @param XYZ color in the [0 1]^3 range.
 * @return sRGB color in the [0 1]^3 range.
 */
vec3 XYZ2rgb(vec3 XYZ) {
    // Conversion matrix for sRGB, D65 white point
    mat3 xyz2rgbD65Mat = mat3( 3.2404542, -0.9692660,  0.0556434,
                              -1.5371385,  1.8760108, -0.2040259,
                              -0.4985314,  0.0415560,  1.0572252);
    vec3 v = xyz2rgbD65Mat * XYZ;
    // sRGB companding
    // The mix function uses step() to efficiently recreate the following code
    // if (v[i] > 0.0031308f) {
    //     rgb[i] = pow(v[i], 1.f/2.4f) * 1.055f - 0.055f;
    // } else {
    //     rgb[i] = v[i] * 12.92f;
    // }
    vec3 rgb = mix(pow(v, vec3(1.0 / 2.4)) * 1.055 - 0.055, v * 12.92, step(v, vec3(0.0031308)));
    return rgb;

}

/** 
 * \brief Convert from sRGB color to XYZ using D65 white point
 * See http://en.wikipedia.org/wiki/CIE_1931_color_space,
 * http://www.brucelindbloom.com/
 * and http://en.wikipedia.org/wiki/RGB_color_model 
 * @param rgb Color in the [0 1]^3 range
 * @return XYZ color in the [0 1]^3 range.
 */
vec3 rgb2XYZ(vec3 rgb) {
    // Conversion matrix for sRGB, D65 white point
    mat3 rgb2xyzD65Mat = mat3( 0.4124564, 0.2126729, 0.0193339,
                               0.3575761, 0.7151522, 0.1191920,
                               0.1804375, 0.0721750, 0.9503041);
    // Inverse sRGB companding
    // The mix function uses step() to efficiently recreate the following code
    // if (rgb[i] > 0.04045) {
    //     v[i] = pow((rgb[i] + 0.055) / 1.055, 2.4);
    // } else {
    //     v[i] = rgb[i] / 12.92;
    // }
    vec3 v = mix(pow((rgb + 0.055) / 1.055, vec3(2.4)), rgb / 12.92, step(rgb, vec3(0.04045)));
    return rgb2xyzD65Mat * v;
}

/**
 * \brief Convert from sRGB color to Lab using D65 white point.
 *
 * See http://en.wikipedia.org/wiki/Lab_color_space
 * http://www.brucelindbloom.com/
 * and http://en.wikipedia.org/wiki/RGB_color_model
 *
 * @param rgb rgb color in the [0 1]^3 range
 * @return Lab color
 */
vec3 rgb2lab(vec3 rgb) {
    vec3 xyz = rgb2XYZ(rgb);
    return XYZ2lab(xyz);
}

/**
 * \brief Convert from Lab color to sRGB using D65 white point.
 *
 * See http://en.wikipedia.org/wiki/Lab_color_space
 * http://www.brucelindbloom.com/
 * and http://en.wikipedia.org/wiki/RGB_color_model
 *
 * @param lab color in Lab color space
 * @return rgb color in the [0 1]^3 range
 */
vec3 lab2rgb(vec3 lab) {
    vec3 xyz = lab2XYZ(lab);
    return XYZ2rgb(xyz);
}

/**
 * \brief Convert from CIE XYZ to CIE Luv
 *
 * The CIE Luv color value is converted to CIE XYZ color space using the reference white point.
 *
 * See http://en.wikipedia.org/wiki/CIELUV, http://en.wikipedia.org/wiki/CIE_1931_color_space, and
 * http://www.brucelindbloom.com/
 *
 * @param XYZ  CIE XYZ color (roughly in the [0 1]^3 range)
 * @param whitePointXYZ  Normalized white point. Default white point is D65.
 * @return CIE Luv color value, L in [0, 100], u and v in [-100, +100] (for typical images)
 */
vec3 XYZ2Luv(vec3 XYZ, vec3 whitePointXYZ = whitePoint) {
    // see http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_Luv.html

    const float epsilon = 216.0 / 24389.0;
    const float kappa = 24389.0 / 27.0;

    // compute u' and v' for reference white point
    // u' <- 4 * X_r / (X_r + 15 * Y_r + 3 * Z_r);
    // v' <- 9 * Y_r / (X_r + 15 * Y_r + 3 * Z_r);
    float u0_prime =
        4 * whitePointXYZ.x / (whitePointXYZ.x + 15 * whitePointXYZ.y + 3 * whitePointXYZ.z);
    float v0_prime =
        9 * whitePointXYZ.y / (whitePointXYZ.x + 15 * whitePointXYZ.y + 3 * whitePointXYZ.z);

    // compute u' and v' for XYZ color value
    float u_prime = 4 * XYZ.x / (XYZ.x + 15 * XYZ.y + 3 * XYZ.z);
    float v_prime = 9 * XYZ.y / (XYZ.x + 15 * XYZ.y + 3 * XYZ.z);

    float yr = XYZ.y / whitePointXYZ.y;
    // L <- ifelse(yr > epsilon, 116 * yr^(1/3) - 16, kappa * yr);
    float L = ((yr > epsilon) ? 116.0 * pow(yr, 1.0 / 3.0) - 16.0 : kappa * yr);
    // u <- 13 * L * (u_prime - u0_prime);
    float u = 13.0 * L * (u_prime - u0_prime);
    // v <- 13 * L * (v_prime - v0_prime);
    float v = 13.0 * L * (v_prime - v0_prime);

    return vec3(L, u, v);
}

/**
 * \brief Convert from CIE Luv to CIE XYZ
 *
 * The CIE Luv color value is converted to CIE XYZ color space using the reference white point.
 *
 * See http://en.wikipedia.org/wiki/CIELUV, http://en.wikipedia.org/wiki/CIE_1931_color_space, and
 * http://www.brucelindbloom.com/
 *
 * @param Luv CIE Luv color, L in [0, 100], u and v in [-100, +100] but values might exceed this
 * range
 * @param whitePointXYZ  Normalized white point. Default white point is D65.
 * @return CIE XYZ color (roughly in the [0 1]^3 range)
 */
vec3 Luv2XYZ(vec3 Luv, vec3 whitePointXYZ = whitePoint) {
    // see http://www.brucelindbloom.com/index.html?Eqn_Luv_to_XYZ.html

    const float epsilon = 216.0 / 24389.0;
    const float kappa = 24389.0 / 27.0;

    // compute u and v for reference white point
    // u0 <- 4 * X_r / (X_r + 15 * Y_r + 3 * Z_r);
    // v0 <- 9 * Y_r / (X_r + 15 * Y_r + 3 * Z_r);
    float u0 =
        4 * whitePointXYZ.x / (whitePointXYZ.x + 15 * whitePointXYZ.y + 3 * whitePointXYZ.z);
    float v0 =
        9 * whitePointXYZ.y / (whitePointXYZ.x + 15 * whitePointXYZ.y + 3 * whitePointXYZ.z);

    float L = Luv.x;
    float u = Luv.y;
    float v = Luv.z;

    // Y <- ifelse(L > kappa * epsilon, ((L + 16) / 116) ^ 3, L / kappa);
    float Y = ((L > kappa * epsilon) ? pow((L + 16) / 116, 3) : L / kappa);

    // a <- 1 / 3 * (52 * L / (u + 13 * L * u0) - 1);
    // b <- -5*Y;
    // c <- -1/3;
    // d <- Y * (39 * L / (v + 13 * L * v0) - 5);
    float a = 1.0 / 3.0 * (52.0 * L / (u + 13 * L * u0) - 1.0);
    float b = -5.0 * Y;
    float c = -1.0 / 3.0;
    float d = Y * (39.0 * L / (v + 13 * L * v0) - 5.0);
    // X <- (d - b) / (a - c);
    float X = (d - b) / (a - c);

    // Z <- X * a + b;
    float Z = X * a + b;

    return vec3(X, Y, Z);
}

/**
 * \brief Convert from CIE XYZ color space to normalized chromaticity of CIE Luv, i.e. u' and v'
 *
 * See http://en.wikipedia.org/wiki/CIELUV, http://www.brucelindbloom.com/
 *
 * @param XYZ CIE XYZ color (roughly in the [0 1]^3 range)
 * @param whitePointXYZ  Normalized white point. Default white point is D65.
 * @return CIE XYZ color (roughly in the [0 1]^3 range)
 */
vec3 XYZ2LuvChromaticity(vec3 XYZ) {
    // see http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_Luv.html
    float epsilon = 0.008856452; // == 216.0 / 24389.0;
    float kappa = 903.2963; // == 24389.0 / 27.0;

    // compute u' and v' for XYZ color value
    float dInv = 1.0 / (XYZ.x + 15 * XYZ.y + 3 * XYZ.z);
    float u_prime = 4 * XYZ.x * dInv; 
    float v_prime = 9 * XYZ.y * dInv;

    float yr = XYZ.y / whitePoint.y;
    // L <- ifelse(yr > epsilon, 116 * yr^(1/3) - 16, kappa * yr);
    float L = ((yr > epsilon) ? 116.0 * pow(yr, 1.0 / 3.0) - 16.0 : kappa * yr);

    return vec3(L, u_prime, v_prime);
}

/**
 * \brief Convert from normalized chromaticity of CIE Luv, i.e. u' and v', to XYZ
 *
 * See http://en.wikipedia.org/wiki/CIELUV, http://www.brucelindbloom.com/
 *
 * @param LuvChroma chromaticity, L in [0, 100], u' in [0.0, 1.0], v' in [0.0, 1.0]
 * @param whitePointXYZ  Normalized white point. Default white point is D65.
 * @return CIE XYZ color (roughly in the [0 1]^3 range)
 */
vec3 LuvChromaticity2XYZ(vec3 LuvChroma,
                         vec3 whitePointXYZ = whitePoint) {
    // compute u and v for reference white point
    // u0 <- 4 * X_r / (X_r + 15 * Y_r + 3 * Z_r);
    // v0 <- 9 * Y_r / (X_r + 15 * Y_r + 3 * Z_r);
    float u0_prime =
        4 * whitePointXYZ.x / (whitePointXYZ.x + 15 * whitePointXYZ.y + 3 * whitePointXYZ.z);
    float v0_prime =
        9 * whitePointXYZ.y / (whitePointXYZ.x + 15 * whitePointXYZ.y + 3 * whitePointXYZ.z);

    float L = LuvChroma.x;
    float u_prime = LuvChroma.y;
    float v_prime = LuvChroma.z;

    // convert chromaticity to CIE Luv
    float u = 13.0 * L * (u_prime - u0_prime);
    float v = 13.0 * L * (v_prime - v0_prime);

    return Luv2XYZ(vec3(L, u, v));
}

/**
 * \brief Convert from sRGB to normalized chromaticity of CIE Luv, i.e. u' and v'
 *
 * The sRGB color is converted to chromaticity of CIE Luv (luminance, u', v') via the CIE XYZ color
 * space.
 *
 * See http://en.wikipedia.org/wiki/CIELUV, http://www.brucelindbloom.com/
 *
 * @param rgb rgb color, [0, 1]^3
 * @param whitePointXYZ  Normalized white point. Default white point is D65.
 * @return LuvChroma chromaticity, L in [0, 100], u' in [0.0, 1.0], v' in [0.0, 1.0]
 */
vec3 rgb2LuvChromaticity(vec3 rgb) {
    return XYZ2LuvChromaticity(rgb2XYZ(rgb));
}

/**
 * \brief Convert from normalized chromaticity of CIE Luv, i.e. u' and v', to sRGB
 *
 * The chromaticity of CIE Luv (luminance, u', v') is converted to sRGB via the CIE XYZ color space.
 *
 * See http://en.wikipedia.org/wiki/CIELUV, http://www.brucelindbloom.com/
 *
 * @param LuvChroma chromaticity, L in [0, 100], u' in [0.0, 1.0], v' in [0.0, 1.0]
 * @param clamp   clamp resulting rgb values to [0,1]^3
 * @param whitePointXYZ  Normalized white point. Default white point is D65.
 * @return rgb color, [0, 1]^3 if clamping is enabled
 */
vec3 LuvChromaticity2rgb(vec3 LuvChroma, bool clamp = false,
                         vec3 whitePointXYZ = whitePoint) {
    vec3 rgb = vec3(XYZ2rgb(LuvChromaticity2XYZ(LuvChroma, whitePointXYZ)));
    if (clamp) {
        // determine largest component
        int index = 0;
        for (int i = 1; i < 3; ++i) {
            if (rgb[i] > rgb[index]) {
                index = i;
            }
        }
        if (rgb[index] > 1.0f) {
            // renormalize rgb values
            rgb /= rgb[index];
        }
        // get rid of negative values
        rgb = max(rgb, vec3(0.0f));
    }
    return rgb;
}

/**
 * \brief Convert from YCbCr color to sRGB
 *
 * The YCbCr color (luminance, blue-yellow chroma, red-green chroma) is
 * converted to sRGB without considering footroom and headroom.
 *
 * See http://en.wikipedia.org/wiki/YCbCr
 *
 * @param ycbcr YCbCr color, Y in [0, 1], Cb in [-0.5, 0.5], Cr in [-0.5, 0.5]
 * @return rgb color, [0, 1]^3
 */
vec3 ycbcr2rgb(vec3 ycbcrColor) {
    vec3 result;
    result.r = ycbcrColor.x + 1.402 * ycbcrColor.z;
    result.g = ycbcrColor.x - 0.344136 * ycbcrColor.y - 0.714136 * ycbcrColor.z;
    result.b = ycbcrColor.x + 1.772 * ycbcrColor.y;

    return clamp(result, 0.0, 1.0);
}

/**
 * \brief Convert from sRGB color to YCbCr
 *
 * The sRGB color is converted to to YCbCr (luminance, blue-yellow chroma,
 * red-green chroma). The YCbCr color uses the full range and does not
 * have a footroom/headroom (0-16 and 236-255).
 *
 * See http://en.wikipedia.org/wiki/YCbCr
 *
 * @param rgb rgb color, [0, 1]^3
 * @return YCbCr color, Y in [0, 1], Cb in [-0.5, 0.5], Cr in [-0.5, 0.5]
 */
vec3 rgb2ycbcr(vec3 rgbColor) {
    vec3 result;
    result.r = calcLuminance(rgbColor);
    result.g = (rgbColor.b - result.r) * 0.565;
    result.b = (rgbColor.r - result.r) * 0.713;

    return result;
}

vec3 rgbToYCbCrStandard(vec3 rgbColor) {
    // ITU-R BT.601
    //const float Kb = 0.114;
    //const float Kr = 0.299;

    // ITU-R BT.709 (HDTV)
    const float Kb = 0.0722;
    const float Kr = 0.2126;

    // ITU-R BT.2020
    //const float Kb = 0.0593;
    //const float Kr = 0.2627;

    vec3 result;

    // luma Y'
    result.r = Kr * rgbColor.r + (1.0 - Kr - Kb) * rgbColor.g + Kb * rgbColor.b; 
    result.g = 0.5 * (rgbColor.b - result.r) / (1 - Kr);
    result.b = 0.5 * (rgbColor.r - result.r) / (1 - Kb);

    return result;
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

#endif  // IVW_COLOR_CONVERSION_GLSL