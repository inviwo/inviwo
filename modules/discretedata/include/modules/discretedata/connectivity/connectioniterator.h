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

#pragma once

#include <modules/discretedata/discretedatamoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/discretedata/discretedatatypes.h>
#include <modules/discretedata/connectivity/elementiterator.h>

namespace inviwo {
namespace discretedata {

class ConnectionRange;
class Connectivity;

/**
 * Iterates over one GridPrimitive type in a Connectivity.
 */
class IVW_MODULE_DISCRETEDATA_API ConnectionIterator {
    friend ConnectionIterator operator+(ind, ConnectionIterator&);
    friend ConnectionIterator operator-(ind, ConnectionIterator&);

public:
    ConnectionIterator(const Connectivity* parent, GridPrimitive dimension,
                       std::shared_ptr<const std::vector<ind>> neighborhood, ind index = 0)
        : toIndex_(index), parent_(parent), toDimension_(dimension), connection_(neighborhood) {}

    ConnectionIterator()
        : toIndex_(-1), parent_(nullptr), toDimension_(GridPrimitive(-1)), connection_() {}

    ~ConnectionIterator() = default;

    ElementIterator operator*() const;

    // Bidirectional iterator
    ConnectionIterator& operator++() {
        toIndex_++;
        return *this;
    }
    ConnectionIterator operator++(int) {
        auto i = *this;
        toIndex_++;
        return i;
    }
    ConnectionIterator& operator--() {
        toIndex_--;
        return *this;
    }
    ConnectionIterator operator--(int) {
        auto i = *this;
        toIndex_--;
        return i;
    }

    // Random access iterator
    ConnectionIterator operator+(ind offset) {
        return ConnectionIterator(parent_, toDimension_, connection_, toIndex_ + offset);
    }
    ConnectionIterator& operator+=(ind offset) {
        toIndex_ += offset;
        return *this;
    }
    ConnectionIterator operator-(ind offset) {
        return ConnectionIterator(parent_, toDimension_, connection_, toIndex_ - offset);
    }
    ConnectionIterator& operator-=(ind offset) {
        toIndex_ -= offset;
        return *this;
    }

    /**
     * Compare. Has false positives with iterators started from different elements but suffices for
     * iteration
     */
    bool operator==(ConnectionIterator& other) {
        return other.parent_ == parent_  // Compare pointers.
               && other.toDimension_ == toDimension_ && other.toIndex_ == toIndex_;
    }
    bool operator!=(ConnectionIterator& other) { return !(other == *this); }

    //! GridPrimitive type the iterator walks through
    GridPrimitive getType() const { return toDimension_; }

    //! The current index. Equivalent to dereferencing.
    ind getIndex() const { return connection_->at(toIndex_); }

    //! Iterate over connected GridPrimitives (neighbors etc)
    ConnectionRange connection(GridPrimitive type) const;

protected:
    //! Index to the current element
    ind toIndex_;

    //! Pointer to Connectivity iterated through - Does not delete
    const Connectivity* parent_;

    //! GridPrimitive type iterated over (0D vertices etc)
    const GridPrimitive toDimension_;

    // TODO: Find better solution. Keeping a ChannelIterator?
    //! List of neighborhood indices
    std::shared_ptr<const std::vector<ind>> connection_;
};

class ConnectionRange {
public:
    ConnectionRange(ind fromIndex, GridPrimitive fromDim, GridPrimitive toDim,
                    const Connectivity* parent);

    ConnectionIterator begin() {
        return ConnectionIterator(parent_, toDimension_, connections_, 0);
    }
    ConnectionIterator end() {
        return ConnectionIterator(parent_, toDimension_, connections_, connections_->size());
    }
    ind size() { return connections_->size(); }

protected:
    const Connectivity* parent_;
    GridPrimitive toDimension_;
    std::shared_ptr<const std::vector<ind>> connections_;
};

}  // namespace discretedata
}  // namespace inviwo
