/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/glmvec.h>

#include <string>
#include <string_view>
#include <cmath>

namespace inviwo::color {

/**
 * \brief reference white point D65 in CIE XYZ color space
 *
 * The values are obtained by converting sRGB(1, 1, 1) to XYZ using rgb2XYZ() since sRGB assumes D65
 * as reference white.
 * See rgb2XYZ, http://www.brucelindbloom.com
 *
 * @return white D65 in XYZ coordinates
 */
constexpr vec3 D65WhitePoint{0.95047f, 1.0f, 1.08883f};

/**
 * \brief convert from hexadecimal html color code to RGBA
 *
 * A hexadecimal html color code is converted to RGBA. Supports both 3 and 6 digit
 * hexcodes with a leading '#' and optional alpha value (single / double digit).
 * In case of 3 respective 6 digits, alpha is implicitly set to 1.0.
 *
 * Supports "#RGB", "#RGBA", "#RRGGBB", "#RRGGBBAA"
 *
 * @param str    html color code in the form of "#10a0b0ff" or "#a0b0c0"
 * @return RGBA color in [0 1]^3 range
 * @throw Exception if string is malformed
 */
IVW_CORE_API vec4 hex2rgba(std::string_view str);

/**
 * \brief convert from rgba to 8-digit hexadecimal html color code
 *
 * RGBA is converted to a 8 digit hexadecimal html color code with leading '#'
 *
 * @param rgba   RGBA color in [0 1]^3 range
 * @return html color code in the form of "#RRGGBBAA"
 */
IVW_CORE_API std::string rgba2hex(const vec4& rgba);

/**
 * \brief convert from rgb to 6-digit hexadecimal html color code
 *
 * RGB is converted to a 6 digit hexadecimal html color code with leading '#'
 *
 * @param rgb   RGB color in [0 1]^3 range
 * @return html color code in the form of "#RRGGBB"
 */
IVW_CORE_API std::string rgb2hex(const vec3& rgb);

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
constexpr vec3 hsv2rgb(const vec3& hsv) {
    double hue = hsv.x;
    const double sat = hsv.y;
    const double val = hsv.z;
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;

    if (sat < 1.0e-8) {  // only value, no saturation
        r = val;
        g = val;
        b = val;
        return {static_cast<float>(r), static_cast<float>(g), static_cast<float>(b)};
    }

    hue *= 360.0;
    // divide hue into six segments, 60 degree each
    const int h_i = static_cast<int>(std::floor(hue / 60.0)) % 6;
    const double f = hue / 60.0 - std::floor(hue / 60.0);
    const double p = val * (1.0 - sat);
    const double q = val * (1.0 - f * sat);
    const double t = val * (1.0 - (1.0 - f) * sat);

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
    return {static_cast<float>(r), static_cast<float>(g), static_cast<float>(b)};
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
constexpr vec3 rgb2hsv(const vec3& rgb) {
    const double r = rgb.x;
    const double g = rgb.y;
    const double b = rgb.z;
    const double val = std::max(std::max(r, g), b);
    double sat = std::min(std::min(r, g), b);
    const double range = val - sat;

    // set hue to zero for undefined values
    const bool notGray = (val - sat > 1.0e-8 || val - sat < -1.0e-8);
    double hue = 0.0;

    // blue hue
    if (notGray) {
        if (b == val)
            hue = 2.0 / 3.0 + 1.0 / 6.0 * (r - g) / range;
        else if (g == val)
            hue = 1.0 / 3.0 + 1.0 / 6.0 * (b - r) / range;
        else if (r == val)
            hue = 1.0 / 6.0 * (g - b) / range;
    }

    if (hue < 0.0) {
        hue += 1.0;
    }
    if (notGray) {
        sat = 1.0 - sat / val;
    } else {
        sat = 0.0;
    }
    return {static_cast<float>(hue), static_cast<float>(sat), static_cast<float>(val)};
}

/**
 * \brief Convert from HSV to RGB color.
 *
 * See http://en.wikipedia.org/wiki/HSL_and_HSV and
 * http://en.wikipedia.org/wiki/RGB_color_model
 * for a detailed explanation of the color spaces.
 *
 * @param hsv Color in the [0 1]^3 range
 * @param hsl Color in the [0 1]^3 range, where [0 1] corresponds to [0 360) degrees for h
 * @return RGB color in [0 1]^3 range
 */
IVW_CORE_API vec3 hsl2rgb(const vec3& hsl);

/**
 * \brief Convert from RGB to HSV color.
 *
 * See http://en.wikipedia.org/wiki/HSL_and_HSV and
 * http://en.wikipedia.org/wiki/RGB_color_model
 * for a detailed explanation of the color spaces.
 *
 * @param rgb Color in the [0 1]^3 range
 * @return HSV color in the [0 1]^3 range
 * @return HSL color in the [0 1]^3 range
 */
IVW_CORE_API vec3 rgb2hsl(const vec3& rgb);

/**
 * \brief Convert from XYZ to Lab color space
 * See http://en.wikipedia.org/wiki/CIE_1931_color_space
 * and http://www.brucelindbloom.com/
 * @param xyz color in the CIE XYZ color space (roughly in the [0 1]^3 range)
 * @param whitePoint Normalized white point. Default white point is D65.
 * @return Lab color
 */
IVW_CORE_API vec3 XYZ2lab(const vec3& xyz, const vec3& whitePoint = D65WhitePoint);

/**
 * \brief Convert from Lab to CIE XYZ color space
 * See http://en.wikipedia.org/wiki/Lab_color_space
 * and http://www.brucelindbloom.com/
 * @param lab Color
 * @param whitePoint Normalized white point. Default white point is D65.
 * @return CIE XYZ color (roughly in the [0 1]^3 range)
 */
IVW_CORE_API vec3 lab2XYZ(const vec3& lab, const vec3& whitePoint = D65WhitePoint);

/**
 * \brief Convert from sRGB color to XYZ using D65 white point
 * See http://en.wikipedia.org/wiki/CIE_1931_color_space,
 * http://www.brucelindbloom.com/
 * and http://en.wikipedia.org/wiki/RGB_color_model
 * @param rgb rgb color in the [0 1]^3 range
 * @return CIE XYZ color (roughly in the [0 1]^3 range)
 */
IVW_CORE_API vec3 rgb2XYZ(const vec3& rgb);

/**
 * \brief Convert from CIE XYZ color to sRGB using D65 white point.
 *
 * See http://en.wikipedia.org/wiki/CIE_1931_color_space,
 * http://www.brucelindbloom.com/
 * and http://en.wikipedia.org/wiki/RGB_color_model
 * @param xyz  CIE XYZ color (roughly in the [0 1]^3 range)
 * @return sRGB color in the [0 1]^3 range
 */
IVW_CORE_API vec3 XYZ2rgb(const vec3& xyz);

/**
 * \brief Convert from CIE XYZ color space to normalized xyY, which is for example used in the
 * chromaticity diagram.
 *
 * See http://en.wikipedia.org/wiki/CIE_1931_color_space and http://www.brucelindbloom.com/
 * @param xyz  CIE XYZ color (roughly in the [0 1]^3 range)
 * @return xyY color, (x, y chromaticity, and Y lightness) in the [0 1]^3 range
 */
IVW_CORE_API vec3 XYZ2xyY(const vec3& xyz);

/**
 * \brief Convert from xyY color space to XYZ
 *
 * See http://en.wikipedia.org/wiki/CIE_1931_color_space and http://www.brucelindbloom.com/
 * @param xyY  xyY color
 * @return CIE XYZ color (roughly in the [0 1]^3 range)
 */
IVW_CORE_API vec3 xyY2XYZ(const vec3& xyY);

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
IVW_CORE_API vec3 rgb2lab(const vec3& rgb);

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
IVW_CORE_API vec3 lab2rgb(const vec3& lab);

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
IVW_CORE_API vec3 rgb2ycbcr(const vec3& rgb);

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
IVW_CORE_API vec3 ycbcr2rgb(const vec3& ycbcr);

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
IVW_CORE_API vec3 LuvChromaticity2rgb(const vec3& LuvChroma, bool clamp = false,
                                      const vec3& whitePointXYZ = D65WhitePoint);

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
IVW_CORE_API vec3 rgb2LuvChromaticity(const vec3& rgb, const vec3& whitePointXYZ = D65WhitePoint);

/**
 * \brief Convert from normalized chromaticity of CIE Luv, i.e. u' and v', to XYZ
 *
 * See http://en.wikipedia.org/wiki/CIELUV, http://www.brucelindbloom.com/
 *
 * @param LuvChroma chromaticity, L in [0, 100], u' in [0.0, 1.0], v' in [0.0, 1.0]
 * @param whitePointXYZ  Normalized white point. Default white point is D65.
 * @return CIE XYZ color (roughly in the [0 1]^3 range)
 */
IVW_CORE_API vec3 LuvChromaticity2XYZ(const vec3& LuvChroma,
                                      const vec3& whitePointXYZ = D65WhitePoint);

/**
 * \brief Convert from CIE XYZ color space to normalized chromaticity of CIE Luv, i.e. u' and v'
 *
 * See http://en.wikipedia.org/wiki/CIELUV, http://www.brucelindbloom.com/
 *
 * @param XYZ CIE XYZ color (roughly in the [0 1]^3 range)
 * @param whitePointXYZ  Normalized white point. Default white point is D65.
 * @return CIE XYZ color (roughly in the [0 1]^3 range)
 */
IVW_CORE_API vec3 XYZ2LuvChromaticity(const vec3& XYZ, const vec3& whitePointXYZ = D65WhitePoint);

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
IVW_CORE_API vec3 XYZ2Luv(const vec3& XYZ, const vec3& whitePointXYZ = D65WhitePoint);

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
IVW_CORE_API vec3 Luv2XYZ(const vec3& Luv, const vec3& whitePointXYZ = D65WhitePoint);

/**
 * \brief Return a lighter color by adjusting the brightness
 *
 * The brightness is modified by multiplying \p rgb input in HSV colorspace with \p factor. A factor
 * > 1.0 will return a lighter color, e.g. a 50% increase is achieved by a factor of 1.5. Instead of
 * using a factor < 1.0, consider using darker(). Factors <= 0 result in unspecified results.
 *
 * @param rgb rgb color, [0, 1]^3
 * @param factor   scaling factor of the brightness
 * @return lighter/darker rgb color
 *
 * See https://doc.qt.io/qt-5/qcolor.html#lighter
 */
constexpr vec3 lighter(const vec3& rgb, float factor = 1.5f) {
    vec3 hsv = rgb2hsv(rgb);
    hsv.z = std::min(hsv.z * factor, 1.0f);
    return hsv2rgb(hsv);
}
/**
 * \overload vec4 lighter(const vec4&, float)
 */
constexpr vec4 lighter(const vec4& rgba, float factor = 1.5f) {
    vec3 hsv = rgb2hsv(rgba);
    hsv.z = std::min(hsv.z * factor, 1.0f);
    return {hsv2rgb(hsv), rgba.a};
}
/**
 * \overload uvec3 lighter(const uvec3&, float)
 */
constexpr uvec3 lighter(const uvec3& rgb, float factor = 1.5f) {
    return uvec3{lighter(vec3{rgb} / 255.0f, factor) * 255.0f};
}
/**
 * \brief Return a darker color by adjusting the brightness
 *
 * The brightness is modified by dividing \p rgb input in HSV colorspace by  \p factor. A factor
 * > 1.0 will return a darker color, e.g. a factor of 2 yields a color with half the brightness.
 * Instead of using a factor < 1.0, consider using darker(). Factors <= 0 result in unspecified
 * results.
 *
 * @param rgb rgb color, [0, 1]^3
 * @param factor   scaling factor of the brightness
 * @return lighter/darker rgb color
 *
 * See https://doc.qt.io/qt-5/qcolor.html#lighter
 */
constexpr vec3 darker(const vec3& rgb, float factor = 2.0f) {
    vec3 hsv = rgb2hsv(rgb);
    hsv.z = std::min(hsv.z / factor, 1.0f);
    return hsv2rgb(hsv);
}
/**
 * \overload vec4 darker(const vec4&, float)
 */
constexpr vec4 darker(const vec4& rgba, float factor = 2.0f) {
    vec3 hsv = rgb2hsv(rgba);
    hsv.z = std::min(hsv.z / factor, 1.0f);
    return {hsv2rgb(hsv), rgba.a};
}
/**
 * \overload uvec3 darker(const uvec3&, float)
 */
constexpr uvec3 darker(const uvec3& rgb, float factor = 2.0f) {
    return uvec3{darker(vec3{rgb} / 255.0f, factor) * 255.0f};
}

}  // namespace inviwo::color
