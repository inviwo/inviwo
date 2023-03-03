/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2023 Inviwo Foundation
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

#include <modules/plotting/plottingmoduledefine.h>  // for IVW_MODULE_PLOTTING...

#include <inviwo/core/properties/compositeproperty.h>                 // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>                 // for InvalidationLevel
#include <inviwo/core/properties/ordinalproperty.h>                   // for FloatProperty, Floa...
#include <inviwo/core/properties/propertysemantics.h>                 // for PropertySemantics
#include <inviwo/core/properties/stringproperty.h>                    // for StringProperty
#include <modules/fontrendering/properties/fontfaceoptionproperty.h>  // for FontFaceOptionProperty

#include <string>       // for string
#include <string_view>  // for string_view
#include <tuple>        // for tie
#include <vector>       // for vector

namespace inviwo {

namespace plot {
class AxisProperty;

/**
 * \brief A convenience property for updating/overriding multiple axes properties. A property change
 * will propagate to all the subproperties of the registered axes.
 */
class IVW_MODULE_PLOTTING_API AxisStyleProperty : public CompositeProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    AxisStyleProperty(std::string_view identifier, std::string_view displayName,
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                      PropertySemantics semantics = PropertySemantics::Default);
    AxisStyleProperty(const AxisStyleProperty& rhs);
    virtual AxisStyleProperty* clone() const override;
    virtual ~AxisStyleProperty() = default;

    void registerProperty(AxisProperty& p);
    template <typename... Ts>
    void registerProperties(Ts&... properties);
    void unregisterProperty(AxisProperty& p);
    void unregisterAll();

    FontFaceOptionProperty fontFace_;
    IntProperty fontSize_;
    FloatVec4Property color_;
    FloatProperty lineWidth_;
    FloatProperty tickLength_;
    StringProperty labelFormat_;

private:
    std::vector<AxisProperty*> axes_;

    auto props() {
        return std::tie(fontFace_, fontSize_, color_, lineWidth_, tickLength_, labelFormat_);
    }
    auto props() const {
        return std::tie(fontFace_, fontSize_, color_, lineWidth_, tickLength_, labelFormat_);
    }
};

template <typename... Ts>
void AxisStyleProperty::registerProperties(Ts&... properties) {
    (registerProperty(properties), ...);
}

}  // namespace plot

}  // namespace inviwo
