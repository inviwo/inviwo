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

#include <modules/discretedata/connectivity/connectioniterator.h>
#include <modules/discretedata/connectivity/connectivity.h>

namespace inviwo {
namespace discretedata {

ConnectionRange::ConnectionRange(ind fromIndex, GridPrimitive fromDim, GridPrimitive toDim,
                                 const Connectivity* parent)
    : parent_(parent), toDimension_(toDim) {
    std::vector<ind>* neigh = new std::vector<ind>();
    parent_->getConnections(*neigh, fromIndex, fromDim, toDim);
    connections_ = std::shared_ptr<const std::vector<ind>>(neigh);
}

ConnectionIterator operator+(ind offset, ConnectionIterator& iter) {
    return ConnectionIterator(iter.parent_, iter.toDimension_, iter.connection_,
                              iter.toIndex_ + offset);
}

ConnectionIterator operator-(ind offset, ConnectionIterator& iter) {
    return ConnectionIterator(iter.parent_, iter.toDimension_, iter.connection_,
                              iter.toIndex_ - offset);
}

ElementIterator ConnectionIterator::operator*() const {
    return ElementIterator(parent_, toDimension_, connection_->at(toIndex_));
}

ConnectionRange ConnectionIterator::connection(GridPrimitive toType) const {
    return ConnectionRange(this->getIndex(), toDimension_, toType, parent_);
}

}  // namespace discretedata
}  // namespace inviwo
