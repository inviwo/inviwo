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

#ifndef IVW_AXISPROPERTY_H
#define IVW_AXISPROPERTY_H

#include <modules/plotting/plottingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>

#include <modules/plotting/properties/tickproperty.h>
#include <modules/plotting/properties/plottextproperty.h>

namespace inviwo {

namespace plot {

class IVW_MODULE_PLOTTING_API AxisProperty : public CompositeProperty {
public:
    InviwoPropertyInfo();

    enum class Orientation { Horizontal, Vertical };
    enum class Placement { Outside, Inside };

    AxisProperty(const std::string& identifier, const std::string& displayName,
                 Orientation orientation = Orientation::Horizontal,
                 InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                 PropertySemantics semantics = PropertySemantics::Default);
    AxisProperty(const AxisProperty& rhs);
    AxisProperty& operator=(const AxisProperty& rhs) = default;
    virtual AxisProperty* clone() const override;
    virtual ~AxisProperty() = default;

    void setTitle(const std::string& title) {
        caption_.title_.set(title);
    }
    void setLabelFormat(const std::string& formatStr) {
        labels_.title_.set(formatStr);
    }
    void setRange(const dvec2& range) { range_.set(range); }

    // general properties
    BoolProperty visible_;
    FloatVec4Property color_;
    FloatProperty width_;
    DoubleMinMaxProperty range_;
    TemplateOptionProperty<Orientation> orientation_;
    TemplateOptionProperty<Placement> placement_;
    // BoolProperty logScale_;

    // caption besides axis
    PlotTextProperty caption_;
    // labels showing numbers along axis
    PlotTextProperty labels_;

    TickProperty ticks_;
};

}  // namespace plot

}  // namespace inviwo

#endif  // IVW_AXISPROPERTY_H
