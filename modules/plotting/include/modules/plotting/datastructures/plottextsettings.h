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
#include <modules/fontrendering/datastructures/fontsettings.h>

namespace inviwo {

namespace plot {

class IVW_MODULE_PLOTTING_API PlotTextSettings {
public:
    PlotTextSettings() = default;
    virtual ~PlotTextSettings() = default;

    virtual bool isEnabled() const = 0;
    virtual vec4 getColor() const = 0;
    virtual float getPosition() const = 0;  //!< position along axis [0,1]
    virtual vec2 getOffset() const = 0;     //!< offset from axis
    virtual float getRotation() const = 0;  //!< Degrees of rotation
    virtual const FontSettings& getFont() const = 0;

    // Conversion to bool for enabled state
    operator bool() const;
};

IVW_MODULE_PLOTTING_API bool operator==(const PlotTextSettings& a, const PlotTextSettings& b);
IVW_MODULE_PLOTTING_API bool operator!=(const PlotTextSettings& a, const PlotTextSettings& b);

}  // namespace plot

}  // namespace inviwo
