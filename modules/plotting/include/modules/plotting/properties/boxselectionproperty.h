/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2025 Inviwo Foundation
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

#include <inviwo/core/properties/compositeproperty.h>              // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>              // for InvalidationLevel, Inv...
#include <inviwo/core/properties/optionproperty.h>                 // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>                // for FloatProperty, FloatVe...
#include <inviwo/core/properties/propertysemantics.h>              // for PropertySemantics, Pro...
#include <inviwo/core/util/glmvec.h>                               // for vec4
#include <inviwo/core/util/staticstring.h>                         // for operator+
#include <modules/plotting/datastructures/boxselectionsettings.h>  // for BoxSelectionSettingsIn...

#include <functional>   // for __base
#include <string>       // for operator==, string
#include <string_view>  // for operator==
#include <vector>       // for operator!=, vector

namespace inviwo {

namespace plot {
/**
 * \brief Settings for rectangle selection/filtering
 *
 */
class IVW_MODULE_PLOTTING_API BoxSelectionProperty : public BoxSelectionSettingsInterface,
                                                     public CompositeProperty {
public:
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.BoxSelectionProperty"};

    BoxSelectionProperty(const std::string& identifier, const std::string& displayName,
                         InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                         PropertySemantics semantics = PropertySemantics::Default);
    BoxSelectionProperty(const BoxSelectionProperty& rhs);
    virtual BoxSelectionProperty* clone() const override;
    virtual ~BoxSelectionProperty() = default;

    OptionProperty<BoxSelectionSettingsInterface::Mode> mode_;
    FloatVec4Property lineColor_;
    FloatProperty lineWidth_;  //!< Line width in pixels

    // Inherited via BoxSelectionSettingsInterface
    /*
     * @copydoc BoxSelectionSettingsInterface::getMode
     */
    virtual BoxSelectionSettingsInterface::Mode getMode() const override;
    /*
     * @copydoc BoxSelectionSettingsInterface::getLineColor
     */
    virtual vec4 getLineColor() const override;
    /*
     * @copydoc BoxSelectionSettingsInterface::getLineWidth
     */
    virtual float getLineWidth() const override;
};

}  // namespace plot

}  // namespace inviwo
