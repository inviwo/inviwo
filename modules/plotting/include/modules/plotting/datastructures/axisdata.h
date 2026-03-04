/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2026 Inviwo Foundation
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
#include <modules/plotting/datastructures/tickdata.h>
#include <modules/plotting/datastructures/textdata.h>
#include <modules/plotting/algorithm/labeling.h>

#include <string>
#include <vector>

namespace inviwo::plot {

struct IVW_MODULE_PLOTTING_API AxisData {
    enum class Orientation : std::uint8_t { Horizontal, Vertical };

    dvec2 range = dvec2{0.0, 100.0};

    bool visible = true;
    bool mirrored = false;
    vec4 color = vec4{0.0f, 0.0f, 0.0f, 1.0f};
    float width = 2.5f;
    float scale = 1.0f;
    Orientation orientation = Orientation::Horizontal;

    std::string caption;
    TextData captionSettings;

    std::vector<double> majorPositions;
    TickData major;

    std::vector<double> minorPositions;
    TickData minor;

    std::vector<std::string> labels;
    TextData labelSettings;

    bool operator==(const AxisData&) const = default;
};

}  // namespace inviwo::plot
