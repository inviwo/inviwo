/*********************************************************************
 *  Author  : Anke Friederici & Tino Weinkauf
 *  Init    : Monday, December 04, 2017 - 11:44:12
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

#include <discretedata/discretedatamoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include "channelgetter.h"

namespace inviwo {
namespace dd {

template <typename T, ind N>
struct ChannelGetter;

/** \struct ChannelIterator
*   Generalized iterator over any DataChannel.
*   Handles data deletion if necessary - runtime check whether needed.
*/
template <typename VecNT, typename T, ind N>
class ChannelIterator {

    static_assert(sizeof(VecNT) == sizeof(T) * N, "Size and type do not agree with the vector type.");

public:

    ChannelIterator(ChannelGetter<T, N>* parent, ind index)
        : Getter(parent), Index(index) {}

    ChannelIterator() : Getter(nullptr), Index(-1), Data(nullptr), DataIndex(-2) {}

    ~ChannelIterator() { delete Getter; }

    /** Dereference to get data */
    VecNT& operator*();

    //*** Bidirectional Iteration ***\\

    /** Walk forward */
    ChannelIterator& operator++() {
        Index++;
        return *this;
    }

    /** Walk backward */
    ChannelIterator& operator--() {
        Index--;
        return *this;
    }

    /** Compare */
    bool operator==(ChannelIterator<VecNT, T, N>& other) {
        return other.Getter->Parent == Getter->Parent
               && other.Index == Index;
    }

    /** Compare */
    bool operator!=(ChannelIterator<VecNT, T, N>& other) { return !(other == *this); }

    //*** Random Access Iteration ***\\

    /** Increment randomly */
    ChannelIterator operator+(ind offset) { return ChannelIterator<VecNT, T, N>(Getter, Index + offset); }

    /** Increment randomly */
    ChannelIterator operator+=(ind offset) { Index += offset; }

    /** Decrement randomly */
    ChannelIterator operator-(ind offset) { return ChannelIterator<VecNT, T, N>(Getter, Index - offset); }

    /** Decrement randomly */
    ChannelIterator operator-=(ind offset) { Index -= offset; }

    // Members
protected:

    /** Abstract struct handling the dereferencing **/
    ChannelGetter<T, N>* Getter;

    /** Index to the current element */
    ind Index;

    /** Pointer to DataChannel iterated through - Do not delete */
//    DataChannel<T, N>* Parent;

    /** Pointer to heap data */
//    VecNT* Data;

    /** Index that is currently pointed to */
//    ind DataIndex;
};

/** Increment randomly */
template <typename VecNT, typename T, ind N>
ChannelIterator<VecNT, T, N> operator+(ind offset, ChannelIterator<VecNT, T, N>& iter) {
    return ChannelIterator(iter.Getter, iter.Index + offset);
}

/** Decrement randomly */
template <typename VecNT, typename T, ind N>
ChannelIterator<VecNT, T, N> operator-(ind offset, ChannelIterator<VecNT, T, N>& iter) {
    return ChannelIterator(iter.Getter, iter.Index - offset);
}

}  // namespace
}

// Circumvent circular reference.
#include "datachannel.h"
#include "bufferchannel.h"

#include "channeliterator.inl"
