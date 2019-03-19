/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

namespace plot {

enum class TickStyle { None, Inside, Outside, Both };

class IVW_MODULE_PLOTTING_API MajorTickSettings {
public:
    MajorTickSettings() = default;
    virtual ~MajorTickSettings() = default;

    virtual TickStyle getStyle() const = 0;
    virtual vec4 getColor() const = 0;
    virtual float getTickLength() const = 0;
    virtual float getTickWidth() const = 0;
    virtual double getTickDelta() const = 0;
    virtual bool getRangeBasedTicks() const = 0;
};
IVW_MODULE_PLOTTING_API bool operator==(const MajorTickSettings& a, const MajorTickSettings& b);
IVW_MODULE_PLOTTING_API bool operator!=(const MajorTickSettings& a, const MajorTickSettings& b);
}  // namespace plot

}  // namespace inviwo
