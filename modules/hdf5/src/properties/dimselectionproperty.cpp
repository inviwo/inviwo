/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/hdf5/properties/dimselectionproperty.h>

#include <inviwo/core/properties/constraintbehavior.h>
#include <inviwo/core/algorithm/markdown.h>

#include <algorithm>

namespace inviwo::hdf5 {

std::string_view DimSelectionProperty::getClassIdentifier() const { return classIdentifier; }

DimSelectionProperty::DimSelectionProperty(std::string_view identifier,
                                           std::string_view displayName,
                                           InvalidationLevel invalidationLevel)
    : CompositeProperty(identifier, displayName, invalidationLevel)
    , start("start", "Start", "First element to read along this dimension"_help, size_t{0},
            {size_t{0}, ConstraintBehavior::Immutable}, {size_t{255}, ConstraintBehavior::Mutable})
    , count("count", "Count", "Number of elements to read, 0 selects all remaining elements"_help,
            size_t{0}, {size_t{0}, ConstraintBehavior::Immutable},
            {size_t{255}, ConstraintBehavior::Mutable})
    , stride("stride", "Stride", "Step between selected elements"_help, size_t{1},
             {size_t{1}, ConstraintBehavior::Immutable},
             {size_t{64}, ConstraintBehavior::Mutable}) {

    addProperties(start, count, stride);
}

DimSelectionProperty::DimSelectionProperty(const DimSelectionProperty& rhs)
    : CompositeProperty(rhs), start(rhs.start), count(rhs.count), stride(rhs.stride) {

    addProperties(start, count, stride);
}

DimSelectionProperty* DimSelectionProperty::clone() const {
    return new DimSelectionProperty(*this);
}

void DimSelectionProperty::update(size_t dimSize) {
    const size_t maxValue = std::max<size_t>(dimSize, 1);
    start.setMaxValue(maxValue);
    count.setMaxValue(maxValue);
}

Selection DimSelectionProperty::getSelection() const {
    return Selection{.start = start.get(), .count = count.get(), .stride = stride.get()};
}

}  // namespace inviwo::hdf5
