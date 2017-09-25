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

#ifndef IVW_TICKPROPERTY_H
#define IVW_TICKPROPERTY_H

#include <modules/plotting/plottingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>

namespace inviwo {

namespace plot {

enum class TickStyle { None, Inside, Outside, Both };

class IVW_MODULE_PLOTTING_API MajorTickProperty : public CompositeProperty {
public:
    InviwoPropertyInfo();

    MajorTickProperty(const std::string& identifier, const std::string& displayName,
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                      PropertySemantics semantics = PropertySemantics::Default);
    MajorTickProperty(const MajorTickProperty& rhs);
    MajorTickProperty& operator=(const MajorTickProperty& rhs) = default;
    virtual MajorTickProperty* clone() const override;
    virtual ~MajorTickProperty() = default;

    TemplateOptionProperty<TickStyle> style_;
    FloatVec4Property color_;
    FloatProperty tickLength_;
    FloatProperty tickWidth_;
    DoubleProperty tickDelta_;
    BoolProperty rangeBasedTicks_;
};

class IVW_MODULE_PLOTTING_API MinorTickProperty : public CompositeProperty {
public:
    InviwoPropertyInfo();

    MinorTickProperty(const std::string& identifier, const std::string& displayName,
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                      PropertySemantics semantics = PropertySemantics::Default);
    MinorTickProperty(const MinorTickProperty& rhs);
    MinorTickProperty& operator=(const MinorTickProperty& rhs) = default;
    virtual MinorTickProperty* clone() const override;
    virtual ~MinorTickProperty() = default;

    TemplateOptionProperty<TickStyle> style_;
    BoolProperty fillAxis_;
    FloatVec4Property color_;
    FloatProperty tickLength_;
    FloatProperty tickWidth_;
    IntProperty tickFrequency_;
};

class IVW_MODULE_PLOTTING_API TickProperty : public CompositeProperty {
public:
    InviwoPropertyInfo();

    TickProperty(const std::string& identifier, const std::string& displayName,
                 InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                 PropertySemantics semantics = PropertySemantics::Default);
    TickProperty(const TickProperty& rhs);
    TickProperty& operator=(const TickProperty& rhs) = default;
    virtual TickProperty* clone() const override;
    virtual ~TickProperty() = default;

    MajorTickProperty majorTicks_;
    MinorTickProperty minorTicks_;
};

}  // namespace plot

}  // namespace inviwo

#endif  // IVW_TICKPROPERTY_H
