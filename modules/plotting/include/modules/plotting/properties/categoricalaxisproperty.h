/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
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

#include <inviwo/core/properties/boolproperty.h>            // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>       // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>       // for InvalidationLevel, Invalidati...
#include <inviwo/core/properties/optionproperty.h>          // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>         // for FloatProperty, FloatVec4Property
#include <inviwo/core/properties/propertysemantics.h>       // for PropertySemantics, PropertySe...
#include <inviwo/core/util/glmvec.h>                        // for dvec2, vec4
#include <inviwo/core/util/staticstring.h>                  // for operator+
#include <modules/plotting/datastructures/axissettings.h>   // for AxisSettings::Orientation
#include <modules/plotting/datastructures/minortickdata.h>  // for MinorTickData
#include <modules/plotting/properties/plottextproperty.h>   // for PlotTextProperty
#include <modules/plotting/properties/tickproperty.h>       // for MajorTickProperty

#include <functional>   // for __base
#include <string>       // for operator==, string, operator+
#include <string_view>  // for operator==, string_view
#include <vector>       // for operator!=, vector, operator==

namespace inviwo {

namespace plot {
class MajorTickSettings;
class MinorTickSettings;
class PlotTextSettings;

/**
 * \brief Axis for variables with a fixed number of possible values, e.g., categories.
 * Will set the AxisProperty::range to match the number of categories and make it read-only.
 * minorTicks will be made invisible.
 */
class IVW_MODULE_PLOTTING_API CategoricalAxisProperty : public AxisSettings,
                                                        public CompositeProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    CategoricalAxisProperty(std::string_view identifier, std::string_view displayName,
                            std::vector<std::string> categories = {"Category"},
                            Orientation orientation = Orientation::Horizontal,
                            InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                            PropertySemantics semantics = PropertySemantics::Default);
    CategoricalAxisProperty(const CategoricalAxisProperty& rhs);
    virtual CategoricalAxisProperty* clone() const override;
    virtual ~CategoricalAxisProperty() = default;

    /*
     * Returns the categories displayed at the major ticks.
     */
    const std::vector<std::string>& getCategories() const;
    /*
     * Sets the categories to display at major ticks and updates the AxisProperty::range property to
     * match the number of categories.
     */
    void setCategories(std::vector<std::string> categories);

    // Inherited via AxisSettings
    virtual dvec2 getRange() const override;
    virtual bool getUseDataRange() const override;

    virtual bool getAxisVisible() const override;
    virtual bool getMirrored() const override;
    virtual vec4 getColor() const override;
    virtual float getWidth() const override;
    virtual float getScalingFactor() const override;
    virtual Orientation getOrientation() const override;

    virtual const std::string& getCaption() const override;
    virtual const PlotTextSettings& getCaptionSettings() const override;

    virtual const std::vector<std::string>& getLabels() const override;
    virtual const PlotTextSettings& getLabelSettings() const override;

    virtual const MajorTickSettings& getMajorTicks() const override;
    virtual const MinorTickSettings& getMinorTicks() const override;

    // general properties
    BoolProperty visible_;
    FloatVec4Property color_;
    FloatProperty width_;
    FloatProperty scalingFactor_;

    BoolProperty mirrored_;
    OptionProperty<Orientation> orientation_;

    // caption besides axis
    PlotTextProperty captionSettings_;
    // labels showing numbers along axis
    PlotTextProperty labelSettings_;

    MajorTickProperty majorTicks_;
    MinorTickData minorTicks_;

private:
    std::vector<std::string> categories_;
    void adjustAlignment();
};

}  // namespace plot

}  // namespace inviwo
