/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/plottinggl/plottingglmoduledefine.h>
#include <inviwo/core/properties/optionproperty.h>

#include <cstdint>

namespace inviwo {

class SpatialEntity;

namespace plot {

enum class AxisRangeMode : unsigned char {
    Dims,
    Basis,
    BasisOffset,
    World,
    DataBoundingBox,
    ModelBoundingBox,
    WorldBoundingBox
};
enum class OffsetScaling : std::uint8_t { None, MinExtent, MaxExtent, MeanExtent, Diagonal };
enum class DimsRangeMode : std::uint8_t { No, Yes };

IVW_MODULE_PLOTTINGGL_API OptionPropertyState<AxisRangeMode> rangeModeState(bool hasDims,
                                                                            bool hasBoundingBox);

IVW_MODULE_PLOTTINGGL_API float calcScaleFactor(const glm::mat4& matrix, OffsetScaling mode);

IVW_MODULE_PLOTTINGGL_API std::array<dvec2, 3> calcAxisRanges(const SpatialEntity& entity,
                                                              std::optional<mat4> worldBoundingBox,
                                                              AxisRangeMode mode);

IVW_MODULE_PLOTTINGGL_API dmat4 getTransform(const SpatialEntity& entity,
                                             std::optional<mat4> worldBoundingBox,
                                             AxisRangeMode mode);

}  // namespace plot
}  // namespace inviwo
