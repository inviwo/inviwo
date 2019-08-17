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

#ifndef IVW_COLORCONVERSION_H
#define IVW_COLORCONVERSION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

namespace color {

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
IVW_CORE_API vec4 hex2rgba(std::string str);

/**
 * \brief convert from rgba to 8-digit hexadecimal html color code
 *
 * RGBA is converted to a 8 digit hexadecimal html color code with leading '#'
 *
 * @param rgba   RGBA color in [0 1]^3 range
 * @return html color code in the form of "#RRGGBBAA"
 */
IVW_CORE_API std::string rgba2hex(const vec4 &rgba);

/**
 * \brief convert from rgb to 6-digit hexadecimal html color code
 *
 * RGB is converted to a 6 digit hexadecimal html color code with leading '#'
 *
 * @param rgb   RGB color in [0 1]^3 range
 * @return html color code in the form of "#RRGGBB"
 */
IVW_CORE_API std::string rgb2hex(const vec3 &rgb);

/**
 * \brief reference white point D65 in CIE XYZ color space
 *
 * The values are obtained by converting sRGB(1, 1, 1) to XYZ using rgb2XYZ() since sRGB assumes D65
 * as reference white.
 * See rgb2XYZ, http://www.brucelindbloom.com
 *
 * @return white D65 in XYZ coordinates
 */
IVW_CORE_API vec3 getD65WhitePoint();

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
IVW_CORE_API vec3 hsv2rgb(vec3 hsv);

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
IVW_CORE_API vec3 rgb2hsv(vec3 rgb);

/**
 * \brief Convert from XYZ to Lab color space
 * See http://en.wikipedia.org/wiki/CIE_1931_color_space
 * and http://www.brucelindbloom.com/
 * @param xyz color in the CIE XYZ color space (roughly in the [0 1]^3 range)
 * @param whitePoint Normalized white point. Default white point is D65.
 * @return Lab color
 */
IVW_CORE_API vec3 XYZ2lab(vec3 xyz, vec3 whitePoint = getD65WhitePoint());

/**
 * \brief Convert from Lab to CIE XYZ color space
 * See http://en.wikipedia.org/wiki/Lab_color_space
 * and http://www.brucelindbloom.com/
 * @param lab Color
 * @param whitePoint Normalized white point. Default white point is D65.
 * @return CIE XYZ color (roughly in the [0 1]^3 range)
 */
IVW_CORE_API vec3 lab2XYZ(vec3 lab, const vec3 whitePoint = getD65WhitePoint());

/**
 * \brief Convert from sRGB color to XYZ using D65 white point
 * See http://en.wikipedia.org/wiki/CIE_1931_color_space,
 * http://www.brucelindbloom.com/
 * and http://en.wikipedia.org/wiki/RGB_color_model
 * @param rgb rgb color in the [0 1]^3 range
 * @return CIE XYZ color (roughly in the [0 1]^3 range)
 */
IVW_CORE_API vec3 rgb2XYZ(const vec3 rgb);

/**
 * \brief Convert from CIE XYZ color to sRGB using D65 white point.
 *
 * See http://en.wikipedia.org/wiki/CIE_1931_color_space,
 * http://www.brucelindbloom.com/
 * and http://en.wikipedia.org/wiki/RGB_color_model
 * @param xyz  CIE XYZ color (roughly in the [0 1]^3 range)
 * @return sRGB color in the [0 1]^3 range
 */
IVW_CORE_API vec3 XYZ2rgb(vec3 xyz);

/**
 * \brief Convert from CIE XYZ color space to normalized xyY, which is for example used in the
 * chromaticity diagram.
 *
 * See http://en.wikipedia.org/wiki/CIE_1931_color_space and http://www.brucelindbloom.com/
 * @param xyz  CIE XYZ color (roughly in the [0 1]^3 range)
 * @return xyY color, (x, y chromaticity, and Y lightness) in the [0 1]^3 range
 */
IVW_CORE_API vec3 XYZ2xyY(vec3 xyz);

/**
 * \brief Convert from xyY color space to XYZ
 *
 * See http://en.wikipedia.org/wiki/CIE_1931_color_space and http://www.brucelindbloom.com/
 * @param xyY  xyY color
 * @return CIE XYZ color (roughly in the [0 1]^3 range)
 */
IVW_CORE_API vec3 xyY2XYZ(vec3 xyY);

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
IVW_CORE_API vec3 rgb2lab(vec3 rgb);

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
IVW_CORE_API vec3 lab2rgb(vec3 lab);

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
IVW_CORE_API vec3 rgb2ycbcr(const vec3 &rgb);

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
IVW_CORE_API vec3 ycbcr2rgb(const vec3 &ycbcr);

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
IVW_CORE_API vec3 LuvChromaticity2rgb(const vec3 &LuvChroma, bool clamp = false,
                                      vec3 whitePointXYZ = getD65WhitePoint());

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
IVW_CORE_API vec3 rgb2LuvChromaticity(const vec3 &rgb, vec3 whitePointXYZ = getD65WhitePoint());

/**
 * \brief Convert from normalized chromaticity of CIE Luv, i.e. u' and v', to XYZ
 *
 * See http://en.wikipedia.org/wiki/CIELUV, http://www.brucelindbloom.com/
 *
 * @param LuvChroma chromaticity, L in [0, 100], u' in [0.0, 1.0], v' in [0.0, 1.0]
 * @param whitePointXYZ  Normalized white point. Default white point is D65.
 * @return CIE XYZ color (roughly in the [0 1]^3 range)
 */
IVW_CORE_API vec3 LuvChromaticity2XYZ(const vec3 &LuvChroma,
                                      vec3 whitePointXYZ = getD65WhitePoint());

/**
 * \brief Convert from CIE XYZ color space to normalized chromaticity of CIE Luv, i.e. u' and v'
 *
 * See http://en.wikipedia.org/wiki/CIELUV, http://www.brucelindbloom.com/
 *
 * @param XYZ CIE XYZ color (roughly in the [0 1]^3 range)
 * @param whitePointXYZ  Normalized white point. Default white point is D65.
 * @return CIE XYZ color (roughly in the [0 1]^3 range)
 */
IVW_CORE_API vec3 XYZ2LuvChromaticity(const vec3 &XYZ, vec3 whitePointXYZ = getD65WhitePoint());

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
IVW_CORE_API vec3 XYZ2Luv(const vec3 &XYZ, vec3 whitePointXYZ = getD65WhitePoint());

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
IVW_CORE_API vec3 Luv2XYZ(const vec3 &Luv, vec3 whitePointXYZ = getD65WhitePoint());

}  // namespace color

}  // namespace inviwo

#endif  // IVW_COLORCONVERSION_H
