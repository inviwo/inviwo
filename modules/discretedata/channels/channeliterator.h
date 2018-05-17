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

namespace inviwo {
namespace dd {

/** \struct ChannelIterator
*   Generalized iterator over any DataChannel.
*   Handles data deletion if necessary - runtime check whether needed.
*/
template <typename T>
class ChannelIterator {
public:
    ChannelIterator(DataChannel<T>* parent, ind index)
        : Parent(parent), Index(index), Data(nullptr), DataIndex(-1) {}

    ChannelIterator() : Parent(nullptr), Index(-1), Data(nullptr), DataIndex(-2) {}

    ~ChannelIterator() { delete[] Data; }

    /** Dereference to get data */
    T* operator*();

    //*** Bidirectional Iteration ***\\

    /** Walk forward */
    ChannelIterator& operator++() {
        Index++;
        return this;
    }

    /** Walk backward */
    ChannelIterator& operator--() {
        Index--;
        return this;
    }

    /** Compare */
    bool operator==(ChannelIterator<T>& other) {
        return other.Parent == Parent  // Compare pointers.
               && other.Index == Index;
    }

    /** Compare */
    bool operator!=(ChannelIterator<T>& other) { return !(other == *this); }

    //*** Random Access Iteration ***\\

    /** Increment randomly */
    ChannelIterator operator+(ind offset) { return ChannelIterator<T>(Parent, Index + offset); }

    /** Increment randomly */
    ChannelIterator operator+=(ind offset) { Index += offset; }

    /** Decrement randomly */
    ChannelIterator operator-(ind offset) { return ChannelIterator<T>(Parent, Index - offset); }

    /** Decrement randomly */
    ChannelIterator operator-=(ind offset) { Index -= offset; }

    // Members
protected:
    /** Index to the current element */
    ind Index;

    /** Pointer to DataChannel iterated through - Do not delete */
    DataChannel<T>* Parent;

    /** Pointer to heap data */
    T* Data;

    /** Index that is currently pointed to */
    ind DataIndex;
};

/** Increment randomly */
template <typename T>
ChannelIterator<T> operator+(ind offset, ChannelIterator<T>& iter) {
    return ChannelIterator(iter.Parent, iter.Index + offset);
}

/** Decrement randomly */
template <typename T>
ChannelIterator<T> operator-(ind offset, ChannelIterator<T>& iter) {
    return ChannelIterator(iter.Parent, iter.Index - offset);
}

}  // namespace
}

// Circumvent circular reference.
#include "datachannel.h"
#include "bufferchannel.h"

#include "channeliterator.inl"
