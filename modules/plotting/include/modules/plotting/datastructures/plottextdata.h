/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <modules/plotting/plottingmoduledefine.h>  // for IVW_MODULE_PLOTTING_API

#include <inviwo/core/util/glmvec.h>                           // for vec2, vec4
#include <modules/fontrendering/datastructures/fontdata.h>     // for FontData
#include <modules/plotting/datastructures/plottextsettings.h>  // for PlotTextSettings

namespace inviwo {
class FontSettings;

namespace plot {

class IVW_MODULE_PLOTTING_API PlotTextData : public PlotTextSettings {
public:
    PlotTextData() = default;
    PlotTextData(const PlotTextSettings& s);
    PlotTextData(const PlotTextData&) = default;
    PlotTextData(PlotTextData&&) noexcept = default;
    PlotTextData& operator=(const PlotTextData&) = default;
    PlotTextData& operator=(PlotTextData&&) = default;
    virtual ~PlotTextData() = default;

    // Inherited via PlotTextSettings
    virtual bool isEnabled() const override;
    virtual LabelPlacement getPlacement() const override;
    virtual vec4 getColor() const override;
    virtual float getPosition() const override;
    virtual vec2 getOffset() const override;
    virtual float getRotation() const override;
    virtual const FontSettings& getFont() const override;

    bool enabled = true;
    LabelPlacement placement = LabelPlacement::Outside;
    vec4 color = vec4{0.0f, 0.0f, 0.0f, 1.0f};
    float position = 0.5f;
    vec2 offset = vec2{10.0f, 0.0f};
    float rotation = 0.0f;
    FontData font;
};

}  // namespace plot

}  // namespace inviwo
