/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#ifndef IVW_COHENSUTHERLAND_H
#define IVW_COHENSUTHERLAND_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <tuple>

namespace inviwo {

namespace algorithm {

/**
 * \brief clip a 2D line at given rectangle using Cohen-Sutherland algorithm
 *
 * This function uses the Cohen-Sutherland algorithm for line clipping. It clips the line between \p
 * p1 and \p p2 at the rectangle defined by the lower left corner \p rectMin and upper right corner
 * \p rectMax.
 *
 * @param p1        starting point of line
 * @param p2        end point of line
 * @param rectMin   lower left corner of rectangle
 * @param rectMax   upper right corner of rectangle
 * @return std::tuple of bool and the clipped line. The bool value is set to false if the line does
 * not intersect with the rectangle, i.e. lies completely outside. In case of an intersection, the
 * bool value is true and the two position describe the clipped line segment inside the rectangle.
 */
IVW_MODULE_BASE_API std::tuple<bool, vec2, vec2> clipLineCohenSutherland(vec2 p1, vec2 p2,
                                                                         const vec2 &rectMin,
                                                                         const vec2 &rectMax);

/**
 * \brief check whether point \p p lies within a rectangle
 *
 * Checks if the point \p p lies inside the rectangle defined by the lower left corner \p rectMin
 * and upper right corner \p rectMax.
 *
 * @param p         point which is checked
 * @param rectMin   lower left corner of rectangle
 * @param rectMax   upper right corner of rectangle
 * @return true if \p point lies inside the rectangle
 */
IVW_MODULE_BASE_API bool insideRect(const vec2 &p, const vec2 &rectMin, const vec2 &rectMax);

}  // namespace algorithm

}  // namespace inviwo

#endif  // IVW_COHENSUTHERLAND_H
