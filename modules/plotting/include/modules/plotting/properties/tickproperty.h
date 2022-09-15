/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2022 Inviwo Foundation
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

#include <modules/plotting/plottingmoduledefine.h>              // for IVW_MODULE_PLOTTING_API

#include <inviwo/core/properties/boolproperty.h>                // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>           // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>           // for InvalidationLevel, Invali...
#include <inviwo/core/properties/optionproperty.h>              // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>             // for FloatProperty, FloatVec4P...
#include <inviwo/core/properties/propertysemantics.h>           // for PropertySemantics, Proper...
#include <inviwo/core/util/glmvec.h>                            // for vec4
#include <inviwo/core/util/staticstring.h>                      // for operator+
#include <modules/plotting/datastructures/majorticksettings.h>  // for TickStyle, MajorTickSettings
#include <modules/plotting/datastructures/minorticksettings.h>  // for MinorTickSettings

#include <functional>                                           // for __base
#include <string>                                               // for operator==, string, opera...
#include <string_view>                                          // for operator==, string_view
#include <vector>                                               // for operator!=, vector, opera...

namespace inviwo {

namespace plot {

class IVW_MODULE_PLOTTING_API MajorTickProperty : public MajorTickSettings,
                                                  public CompositeProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    MajorTickProperty(std::string_view identifier, std::string_view displayName,
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                      PropertySemantics semantics = PropertySemantics::Default);
    MajorTickProperty(const MajorTickProperty& rhs);
    virtual MajorTickProperty* clone() const override;
    virtual ~MajorTickProperty() = default;

    OptionProperty<TickStyle> style_;
    FloatVec4Property color_;
    FloatProperty tickLength_;
    FloatProperty tickWidth_;
    DoubleProperty tickDelta_;
    BoolProperty rangeBasedTicks_;

    // Inherited via MajorTickSettings
    virtual TickStyle getStyle() const override;
    virtual vec4 getColor() const override;
    virtual float getTickLength() const override;
    virtual float getTickWidth() const override;
    virtual double getTickDelta() const override;
    virtual bool getRangeBasedTicks() const override;
};

class IVW_MODULE_PLOTTING_API MinorTickProperty : public MinorTickSettings,
                                                  public CompositeProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    MinorTickProperty(std::string_view identifier, std::string_view displayName,
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                      PropertySemantics semantics = PropertySemantics::Default);
    MinorTickProperty(const MinorTickProperty& rhs);
    virtual MinorTickProperty* clone() const override;
    virtual ~MinorTickProperty() = default;

    OptionProperty<TickStyle> style_;
    BoolProperty fillAxis_;
    FloatVec4Property color_;
    FloatProperty tickLength_;
    FloatProperty tickWidth_;
    IntProperty tickFrequency_;

    // Inherited via MinorTickSettings
    virtual TickStyle getStyle() const override;
    virtual bool getFillAxis() const override;
    virtual vec4 getColor() const override;
    virtual float getTickLength() const override;
    virtual float getTickWidth() const override;
    virtual int getTickFrequency() const override;
};

class IVW_MODULE_PLOTTING_API TickProperty : public CompositeProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    TickProperty(std::string_view identifier, std::string_view displayName,
                 InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                 PropertySemantics semantics = PropertySemantics::Default);
    TickProperty(const TickProperty& rhs);
    virtual TickProperty* clone() const override;
    virtual ~TickProperty() = default;

    MajorTickProperty majorTicks_;
    MinorTickProperty minorTicks_;
};

}  // namespace plot

}  // namespace inviwo
