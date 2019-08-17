/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_PLOTTEXTPROPERTY_H
#define IVW_PLOTTEXTPROPERTY_H

#include <modules/plotting/plottingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <modules/fontrendering/properties/fontproperty.h>
#include <modules/plotting/datastructures/plottextsettings.h>

namespace inviwo {

namespace plot {

class IVW_MODULE_PLOTTING_API PlotTextProperty : public PlotTextSettings,
                                                 public BoolCompositeProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    PlotTextProperty(const std::string& identifier, const std::string& displayName,
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

#endif  // IVW_PLOTTEXTPROPERTY_H
