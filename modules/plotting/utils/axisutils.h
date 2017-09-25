/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#ifndef IVW_AXISUTILS_H
#define IVW_AXISUTILS_H

#include <modules/plotting/plottingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/plotting/properties/axisproperty.h>

#include <tuple>

namespace inviwo {

class Mesh;

namespace plot {

/**
 * \brief returns tick positions along the axis range defined in the axis property
 *
 * @param property   axis property used for tick settings
 * @return positions of tick marks
 */
std::vector<double> IVW_MODULE_PLOTTING_API getMajorTickPositions(const AxisProperty& property);
std::vector<double> IVW_MODULE_PLOTTING_API getMinorTickPositions(const AxisProperty& property);

vec2 IVW_MODULE_PLOTTING_API getAxisCaptionPosition(const AxisProperty& property,
                                                    const vec2& startPos, const vec2& endPos);

std::vector<std::pair<double, vec2>> IVW_MODULE_PLOTTING_API
getLabelPositions(const AxisProperty& property, const vec2& startPos, const vec2& endPos);

vec3 IVW_MODULE_PLOTTING_API getAxisCaptionPosition3D(const AxisProperty& property,
                                                      const vec3& startPos, const vec3& endPos,
                                                      const vec3& tickDirection);

std::vector<std::pair<double, vec3>> IVW_MODULE_PLOTTING_API
getLabelPositions3D(const AxisProperty& property, const vec3& startPos, const vec3& endPos,
                    const vec3& tickDirection);

/**
 * \brief creates a mesh containing major ticks for an axis extending from start to
 * end position
 *
 * @param property    axis property used for tick settings
 * @param outputDim   dimensions of the canvas
 * @param startPos  start position of axis
 * @param endPos    end position of axis
 * @return mesh containing of ticks, each tick is represented by two positions and matching colors
 */
std::shared_ptr<Mesh> IVW_MODULE_PLOTTING_API generateMajorTicksMesh(const AxisProperty& property,
                                                                     const vec2& startPos,
                                                                     const vec2& endPos);

std::shared_ptr<Mesh> IVW_MODULE_PLOTTING_API generateMinorTicksMesh(const AxisProperty& property,
                                                                     const vec2& startPos,
                                                                     const vec2& endPos);

std::shared_ptr<Mesh> IVW_MODULE_PLOTTING_API generateAxisMesh(const AxisProperty& property,
                                                               const vec2& startPos,
                                                               const vec2& endPos);

std::shared_ptr<Mesh> IVW_MODULE_PLOTTING_API generateMajorTicksMesh3D(const AxisProperty& property,
                                                                       const vec3& startPos,
                                                                       const vec3& endPos,
                                                                       const vec3& tickDirection);

std::shared_ptr<Mesh> IVW_MODULE_PLOTTING_API generateMinorTicksMesh3D(const AxisProperty& property,
                                                                       const vec3& startPos,
                                                                       const vec3& endPos,
                                                                       const vec3& tickDirection);

std::shared_ptr<Mesh> IVW_MODULE_PLOTTING_API generateAxisMesh3D(const AxisProperty& property,
                                                                 const vec3& startPos,
                                                                 const vec3& endPos);

/**
 * \brief utility function to create a mesh for a given set of tick mark positions
 *
 * @param tickMarks positions of tick marks in same coordinate system as the axis range
 * @param axisRange min/max values of the axis
 * @param startPos  start position of axis (spatial coordinates)
 * @param endPos    end position of axis  (spatial coordinates)
 * @param tickDirection   direction of ticks pointing outwards (spatial coordinates)
 * @param style     tick style (none, inside, outside, both)
 * @param color     tick color
 * @param flip      if true, the orientation of ticks is flipped (only affects inside/outside
 *                  ticks). This is, e.g., used for axis at the top (default tick outside
 *                  location is bottom/left)
 * @return mesh containing of ticks, each tick is represented by two positions and matching colors
 */
std::shared_ptr<Mesh> IVW_MODULE_PLOTTING_API generateTicksMesh(
    const std::vector<double> tickMarks, dvec2 axisRange, const vec3& startPos, const vec3& endPos,
    const vec3& tickDirection, float tickLength, TickStyle style, const vec4& color, bool flip);

}  // namespace plotting

}  // namespace inviwo

#endif  // IVW_AXISUTILS_H
