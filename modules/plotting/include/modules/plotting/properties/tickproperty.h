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

#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/util/glmvec.h>
#include <modules/plotting/datastructures/tickdata.h>

#include <string>
#include <string_view>
#include <vector>

namespace inviwo::plot {

class IVW_MODULE_PLOTTING_API MajorTickProperty : public CompositeProperty {
public:
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.MajorTickProperty"};

    MajorTickProperty(std::string_view identifier, std::string_view displayName,
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                      PropertySemantics semantics = PropertySemantics::Default);
    MajorTickProperty(const MajorTickProperty& rhs);
    virtual MajorTickProperty* clone() const override;
    virtual ~MajorTickProperty() = default;

    OptionProperty<TickData::Style> style;
    FloatVec4Property color;
    FloatProperty length;
    FloatProperty width;
    IntProperty numberOfTicks;

    void update(TickData& data) const;
};

class IVW_MODULE_PLOTTING_API MinorTickProperty : public CompositeProperty {
public:
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.MinorTickProperty"};

    MinorTickProperty(std::string_view identifier, std::string_view displayName,
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                      PropertySemantics semantics = PropertySemantics::Default);
    MinorTickProperty(const MinorTickProperty& rhs);
    virtual MinorTickProperty* clone() const override;
    virtual ~MinorTickProperty() = default;

    OptionProperty<TickData::Style> style;
    BoolProperty fillAxis;
    FloatVec4Property color;
    FloatProperty length;
    FloatProperty width;
    IntProperty frequency;

    void update(TickData& data) const;
};

}  // namespace inviwo::plot
