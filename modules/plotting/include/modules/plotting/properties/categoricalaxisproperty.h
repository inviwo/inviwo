/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
#include <inviwo/core/common/inviwo.h>
#include <modules/plotting/properties/axisproperty.h>

namespace inviwo {

namespace plot {
/**
 * \brief Axis for variables with a fixed number of possible values, e.g., categories.
 * Will set the AxisProperty::range to match the number of categories and make it read-only.
 * minorTicks will be made invisible.
 */
class IVW_MODULE_PLOTTING_API CategoricalAxisProperty : public AxisProperty {
public: 
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    CategoricalAxisProperty(const std::string& identifier, const std::string& displayName,
                            std::vector<std::string> categories = {"Category"},
                 Orientation orientation = Orientation::Horizontal,
                 InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                 PropertySemantics semantics = PropertySemantics::Default);
    CategoricalAxisProperty(const CategoricalAxisProperty& rhs);
    CategoricalAxisProperty& operator=(const CategoricalAxisProperty& rhs) = default;
    virtual CategoricalAxisProperty* clone() const override;
    virtual ~CategoricalAxisProperty() = default;

    /* 
     * Returns the categories displayed at the major ticks.
     */
    const std::vector<std::string>& getCategories() const { return categories_; }
    /*
     * Sets the categories to display at major ticks and updates the AxisProperty::range property to
     * match the number of categories.
     */
    void setCategories(std::vector<std::string> categories);

protected:
    std::vector<std::string> categories_;
};

}  // namespace plot

}  // namespace inviwo
