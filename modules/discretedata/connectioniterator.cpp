/*********************************************************************
*  Author  : Anke Friederici and Tino Weinkauf
*  Init    : Friday, December 15, 2017 - 16:42:19
*
*  Project : KTH Inviwo Modules
*
*  License : Follows the Inviwo BSD license model
*********************************************************************
*/

#include "discretedata/connectioniterator.h"

namespace inviwo {
namespace dd {

ConnectionRange::ConnectionRange(ind fromIndex, GridPrimitive fromDim, GridPrimitive toDim,
                                 const Connectivity* parent)
    : parent_(parent), toDimension_(toDim) {
    connections_ =
        std::make_shared<const std::vector<ind>>(parent_->getConnections(fromIndex, fromDim, toDim));
}

/** Increment randomly */
ConnectionIterator operator+(ind offset, ConnectionIterator& iter) {
    return ConnectionIterator(iter.parent_, iter.toDimension_, iter.connection_,
                              iter.toIndex_ + offset);
}

/** Decrement randomly */
ConnectionIterator operator-(ind offset, ConnectionIterator& iter) {
    return ConnectionIterator(iter.parent_, iter.toDimension_, iter.connection_,
                              iter.toIndex_ - offset);
}

ElementIterator ConnectionIterator::operator*() const {
    ivwAssert(Parent, "No channel to iterate is set.");

    return ElementIterator(parent_, toDimension_, connection_->at(toIndex_));
}

ConnectionRange ConnectionIterator::connection(GridPrimitive toType) const {
    return ConnectionRange(this->getIndex(), toDimension_, toType, parent_);
}

}  // namespace
}