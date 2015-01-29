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
IVW_CORE_API inline vec3 hsv2rgb(const vec3& hsv)
{
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
IVW_CORE_API inline vec3 rgb2hsv(const vec3& rgb)
{
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

    if (sat != 0.f) {
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

#endif // IVW_COLORCONVERSION_H
