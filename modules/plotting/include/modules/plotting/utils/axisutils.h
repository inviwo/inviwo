/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2026 Inviwo Foundation
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

#include <modules/plotting/plottingmoduledefine.h>

#include <inviwo/core/util/glmvec.h>
#include <modules/plotting/datastructures/axisdata.h>
#include <modules/plotting/datastructures/tickdata.h>

#include <cstddef>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

namespace inviwo {

class Mesh;

namespace plot {

/**
 * No picking buffer will be generated if pickingId = std::numeric_limits<size_t>::max()
 */
IVW_MODULE_PLOTTING_API std::unique_ptr<Mesh> generateAxisMesh3D(
    const vec3& startPos, const vec3& endPos,
    const size_t& pickingId = std::numeric_limits<size_t>::max());

IVW_MODULE_PLOTTING_API std::vector<std::pair<double, vec2>> getLabelPositions(
    const std::vector<double>& ticks, const AxisData& data, const vec2& startPos,
    const vec2& endPos);

IVW_MODULE_PLOTTING_API std::vector<std::pair<double, vec3>> getLabelPositions3D(
    const std::vector<double>& positions, const AxisData& data, const vec3& startPos,
    const vec3& endPos, const vec3& tickDirection);

IVW_MODULE_PLOTTING_API vec2 getAxisCaptionPosition(const AxisData& data, const vec2& startPos,
                                                    const vec2& endPos);
IVW_MODULE_PLOTTING_API vec3 getAxisCaptionPosition3D(const AxisData& data, const vec3& startPos,
                                                      const vec3& endPos,
                                                      const vec3& tickDirection);

IVW_MODULE_PLOTTING_API std::pair<vec2, vec2> tickBoundingRect(
    const AxisData& data, const std::vector<double>& majorPositions,
    const std::vector<double>& minorPositions, const vec2& startPos, const vec2& endPos);

/**
 * @brief utility function to create a mesh for a given set of tick mark positions
 *
 * @param tickMarks positions of tick marks in same coordinate system as the axis range
 * @param axisRange min/max values of the axis
 * @param style     tick style (none, inside, outside, both)

 * @return mesh containing of ticks, each tick is represented by two positions and matching
 * colors
 */
IVW_MODULE_PLOTTING_API std::unique_ptr<Mesh> generateTicksMesh(
    const std::vector<double>& tickMarks, dvec2 axisRange, TickData::Style style);

}  // namespace plot

}  // namespace inviwo
