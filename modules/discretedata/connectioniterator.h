/*********************************************************************
*  Author  : Anke Friederici and Tino Weinkauf
*  Init    : Friday, December 15, 2017 - 16:42:19
*
*  Project : KTH Inviwo Modules
*
*  License : Follows the Inviwo BSD license model
*********************************************************************
*/

#pragma once

#include <discretedata/discretedatamoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include "datachannel.h"
#include "connectivity.h"
#include "elementiterator.h"

namespace inviwo {
namespace dd {

class ConnectionRange;

/** \class ConnectionIterator
*   Iterates over one GridPrimitive type in a Connectivity.
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

    /** Dereference to 'get data' */
    ElementIterator operator*() const;

    //*** Bidirectional Iteration ***\\

    /** Walk forward */
    ConnectionIterator& operator++() {
        toIndex_++;
        return *this;
    }

    /** Walk backward */
    ConnectionIterator& operator--() {
        toIndex_--;
        return *this;
    }

    /** Compare. Has false positives with iterators started from different elements but suffices for
     * iteration */
    bool operator==(ConnectionIterator& other) {
        return other.parent_ == parent_  // Compare pointers.
               && other.toDimension_ == toDimension_ && other.toIndex_ == toIndex_;
    }

    /** Compare */
    bool operator!=(ConnectionIterator& other) { return !(other == *this); }

    //*** Random Access Iteration ***\\

    /** Increment randomly */
    ConnectionIterator operator+(ind offset) {
        return ConnectionIterator(parent_, toDimension_, connection_, toIndex_ + offset);
    }

    /** Increment randomly */
    ConnectionIterator& operator+=(ind offset) {
        toIndex_ += offset;
        return *this;
    }

    /** Decrement randomly */
    ConnectionIterator operator-(ind offset) {
        return ConnectionIterator(parent_, toDimension_, connection_, toIndex_ - offset);
    }

    /** Decrement randomly */
    ConnectionIterator& operator-=(ind offset) {
        toIndex_ -= offset;
        return *this;
    }

    /** GridPrimitive type the iterator walks through */
    GridPrimitive getType() const { return toDimension_; }

    /** The current index. Equivalent to dereferencing. */
    ind getIndex() const { return connection_->at(toIndex_); }

    /** Iterate over connected GridPrimitives (neighbors etc) */
    ConnectionRange connection(GridPrimitive type) const;

    // Members
protected:
    /** Index to the current element */
    ind toIndex_;

    /** Pointer to Connectivity iterated through - Does not delete */
    const Connectivity* parent_;

    /** GridPrimitive type iterated over (0D vertices etc) */
    const GridPrimitive toDimension_;

    // TODO: Find better solution. Keeping a ChannelIterator?
    /** List of neighborhood indices */
    std::shared_ptr<const std::vector<ind>> connection_;
};

class ConnectionRange {
public:
    ConnectionRange(ind fromIndex, GridPrimitive fromDim, GridPrimitive toDim,
                    const Connectivity* parent);

    ConnectionIterator begin() { return ConnectionIterator(parent_, toDimension_, connections_, 0); }
    ConnectionIterator end() {
        return ConnectionIterator(parent_, toDimension_, connections_, connections_->size());
    }

protected:
    const Connectivity* parent_;
    GridPrimitive toDimension_;
    std::shared_ptr<const std::vector<ind>> connections_;
};

}  // namespace
}
