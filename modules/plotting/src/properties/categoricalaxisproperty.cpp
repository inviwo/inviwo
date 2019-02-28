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

#include <modules/plotting/properties/categoricalaxisproperty.h>

namespace inviwo {

namespace plot {

const std::string CategoricalAxisProperty::classIdentifier = "org.inviwo.CategoricalAxisProperty";
std::string CategoricalAxisProperty::getClassIdentifier() const { return classIdentifier; }

CategoricalAxisProperty::CategoricalAxisProperty(const std::string& identifier,
                                                 const std::string& displayName,
                                                 std::vector<std::string> categories,
                                                 Orientation orientation,
                                                 InvalidationLevel invalidationLevel,
                                                 PropertySemantics semantics)
    : AxisProperty(identifier, displayName, orientation, invalidationLevel, semantics) {
    setCategories(categories);
    ticks_.majorTicks_.tickDelta_.set(1.0);
    ticks_.majorTicks_.tickDelta_.setReadOnly(true);
    ticks_.minorTicks_.tickFrequency_.set(0, 0, 1, 1);
    ticks_.minorTicks_.setVisible(false);
}

CategoricalAxisProperty::CategoricalAxisProperty(const CategoricalAxisProperty& rhs)
    : AxisProperty(rhs) {

    
}

CategoricalAxisProperty* CategoricalAxisProperty::clone() const {
    return new CategoricalAxisProperty(*this);
}

const std::vector<std::string>& CategoricalAxisProperty::getCategories() const {
  return categories_;
}

void CategoricalAxisProperty::setCategories(std::vector<std::string> categories) {
    categories_ = categories;
    range_.set({0, static_cast<double>(categories_.size() - 1)},
               {0, static_cast<double>(categories_.size() - 1)}, 1.0, 0);
}
}  // namespace plot


}  // namespace inviwo
