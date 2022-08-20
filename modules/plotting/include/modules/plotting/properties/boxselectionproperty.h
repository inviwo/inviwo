/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2022 Inviwo Foundation
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
#include <modules/plotting/datastructures/boxselectionsettings.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>

#include <inviwo/core/util/glmvec.h>

namespace inviwo {

namespace plot {
/**
 * \brief Settings for rectangle selection/filtering
 *
 */
class IVW_MODULE_PLOTTING_API BoxSelectionProperty : public BoxSelectionSettingsInterface,
                                                     public CompositeProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

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
