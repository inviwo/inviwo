/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <modules/discretedata/connectivity/elementiterator.h>
#include <modules/discretedata/connectivity/connectioniterator.h>
#include <modules/discretedata/connectivity/connectivity.h>

namespace inviwo {
namespace discretedata {

ElementIterator::ElementIterator(const Connectivity* parent, GridPrimitive dimension, ind index)
    : index_(index), parent_(parent), dimension_(dimension) {}

ElementIterator::ElementIterator() : ElementIterator(nullptr, GridPrimitive(-1), -1) {}

ElementIterator operator+(ind offset, ElementIterator& iter) {
    return ElementIterator(iter.parent_, iter.dimension_, iter.index_ + offset);
}

ElementIterator operator-(ind offset, ElementIterator& iter) {
    return ElementIterator(iter.parent_, iter.dimension_, iter.index_ - offset);
}

ElementIterator& ElementIterator::operator*() {
    assert(parent_ && "No channel to iterate is set.");
    return *this;
}

ConnectionRange ElementIterator::connection(GridPrimitive toType) const {
    return ConnectionRange(index_, dimension_, toType, parent_);
}

ElementIterator ElementRange::begin() { return ElementIterator(parent_, dimension_, 0); }

ElementIterator ElementRange::end() {
    return ElementIterator(parent_, dimension_, parent_->getNumElements(dimension_));
}

}  // namespace discretedata
}  // namespace inviwo
