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

namespace inviwo {

namespace hdf5 {

std::string_view DimSelectionProperty::getClassIdentifier() const { return classIdentifier; }

DimSelectionProperty::DimSelectionProperty(std::string_view identifier,
                                           std::string_view displayName,
                                           InvalidationLevel invalidationLevel)
    : CompositeProperty(identifier, displayName, invalidationLevel)
    , start_("start", "Start", "First element to read along this dimension"_help, size_t{0},
             {size_t{0}, ConstraintBehavior::Immutable}, {size_t{255}, ConstraintBehavior::Mutable})
    , count_("count", "Count", "Number of elements to read, 0 selects all remaining elements"_help,
             size_t{0}, {size_t{0}, ConstraintBehavior::Immutable},
             {size_t{255}, ConstraintBehavior::Mutable})
    , stride_("stride", "Stride", "Step between selected elements"_help, size_t{1},
              {size_t{1}, ConstraintBehavior::Immutable},
              {size_t{255}, ConstraintBehavior::Mutable}) {

    addProperties(start_, count_, stride_);
}

DimSelectionProperty::DimSelectionProperty(const DimSelectionProperty& rhs)
    : CompositeProperty(rhs)
    , start_(rhs.start_)
    , count_(rhs.count_)
    , stride_(rhs.stride_)
    , dimSize_(rhs.dimSize_) {

    addProperties(start_, count_, stride_);
}

DimSelectionProperty* DimSelectionProperty::clone() const {
    return new DimSelectionProperty(*this);
}

void DimSelectionProperty::update(size_t dimSize) {
    dimSize_ = dimSize;
    const size_t maxValue = std::max<size_t>(dimSize, 1);
    start_.setMaxValue(maxValue);
    count_.setMaxValue(maxValue);
    stride_.setMaxValue(maxValue);
}

Selection DimSelectionProperty::getSelection() const {
    const size_t stride = std::max<size_t>(stride_.get(), 1);
    const size_t start = std::min(start_.get(), dimSize_);
    const size_t count = count_.get();
    const size_t end = (count == 0) ? dimSize_ : std::min(start + count * stride, dimSize_);
    return Selection{start, end, stride};
}

Selection DimSelectionProperty::getMaxSelection() const { return Selection{0, dimSize_, 1}; }

}  // namespace hdf5

}  // namespace inviwo
