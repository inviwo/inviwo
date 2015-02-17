/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#ifndef IVW_COLORCONVERSION_H
#define IVW_COLORCONVERSION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

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
IVW_CORE_API vec3 hsv2rgb(vec3 hsv);

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
IVW_CORE_API vec3 rgb2hsv(vec3 rgb);

/** 
 * \brief Convert from XYZ to Lab color space
 * See http://en.wikipedia.org/wiki/CIE_1931_color_space
 * and http://www.brucelindbloom.com/
 * @param xyz Color in the [0 1]^3 range
 * @param whitePoint Normalized white point. Default white point is D65.
 * @return vec3 Color in Lab space 
 */
IVW_CORE_API vec3 xyz2lab(vec3 xyz, vec3 whitePoint = vec3(0.95047f, 1.f, 1.08883f));

/** 
 * \brief Convert from Lab to XYZ color space
 * See http://en.wikipedia.org/wiki/Lab_color_space
 * and http://www.brucelindbloom.com/
 * @param lab Color
 * @param whitePoint Normalized white point. Default white point is D65.
 * @return Lab color 
 */
IVW_CORE_API vec3 lab2xyz(vec3 lab, const vec3 whitePoint = vec3(0.95047f, 1.f, 1.08883f));

/** 
 * \brief Convert from sRGB color to XYZ using D65 white point
 * See http://en.wikipedia.org/wiki/CIE_1931_color_space,
 * http://www.brucelindbloom.com/
 * and http://en.wikipedia.org/wiki/RGB_color_model 
 * @param rgb Color in the [0 1]^3 range
 * @return XYZ color in the [0 1]^3 range.
 */
IVW_CORE_API vec3 rgb2xyz(const vec3 rgb);

/** 
 * \brief Convert from XYZ color to sRGB using D65 white point.
 *
 * See http://en.wikipedia.org/wiki/CIE_1931_color_space,
 * http://www.brucelindbloom.com/
 * and http://en.wikipedia.org/wiki/RGB_color_model 
 * @param xyz color in the [0 1]^3 range.
 * @return sRGB color in the [0 1]^3 range.
 */
IVW_CORE_API vec3 xyz2rgb(vec3 xyz);

/** 
 * \brief Convert from sRGB color to Lab using D65 white point.
 *
 * See http://en.wikipedia.org/wiki/Lab_color_space
 * http://www.brucelindbloom.com/
 * and http://en.wikipedia.org/wiki/RGB_color_model 
 * 
 * @param rgb Color in the [0 1]^3 range.
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
 * @param lab 
 * @return Color in the [0 1]^3 range.
 */
IVW_CORE_API vec3 lab2rgb(vec3 lab);

}

#endif // IVW_COLORCONVERSION_H
