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

#include <modules/plotting/plottingmoduledefine.h>  // for IVW_MODULE_PLOTTING_API

#include <inviwo/core/properties/boolcompositeproperty.h>      // for BoolCompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>          // for InvalidationLevel, Invalid...
#include <inviwo/core/properties/ordinalproperty.h>            // for FloatProperty, FloatVec4Pr...
#include <inviwo/core/properties/propertysemantics.h>          // for PropertySemantics, Propert...
#include <inviwo/core/properties/stringproperty.h>             // for StringProperty
#include <inviwo/core/util/glmvec.h>                           // for vec2, vec4
#include <modules/fontrendering/properties/fontproperty.h>     // for FontProperty
#include <modules/plotting/datastructures/plottextsettings.h>  // for PlotTextSettings

#include <string>       // for string
#include <string_view>  // for string_view

namespace inviwo {
class FontSettings;

namespace plot {

class IVW_MODULE_PLOTTING_API PlotTextProperty : public PlotTextSettings,
                                                 public BoolCompositeProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    PlotTextProperty(std::string_view identifier, std::string_view displayName,
                     bool checked = false,
                     InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                     PropertySemantics semantics = PropertySemantics::Default);
    PlotTextProperty(const PlotTextProperty& rhs);
    virtual PlotTextProperty* clone() const override;
    virtual ~PlotTextProperty() = default;

    StringProperty title_;
    FloatVec4Property color_;
    FloatProperty position_;  //!< position along axis [0,1]
    FloatProperty offset_;    //!< offset from axis
    FloatProperty rotation_;  //!< Counter-clockwise rotation in degrees,
                              //!  0 degrees means horizontal orientation
    FontProperty font_;

    // Inherited via PlotTextSettings
    virtual bool isEnabled() const override;
    virtual vec4 getColor() const override;
    virtual float getPosition() const override;
    virtual vec2 getOffset() const override;
    virtual float getRotation() const override;
    virtual const FontSettings& getFont() const override;
};

}  // namespace plot

}  // namespace inviwo
