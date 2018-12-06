/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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

#include <modules/discretedata/channels/channelgetter.h>

namespace inviwo {
namespace discretedata {

template <typename T, ind N>
struct ChannelGetter;

/** \struct ChannelIterator
 *   Generalized iterator over any DataChannel.
 *   Handles data deletion if necessary - runtime check whether needed.
 */
template <typename VecNT, typename T, ind N>
class ChannelIterator {

    static_assert(sizeof(VecNT) == sizeof(T) * N,
                  "Size and type do not agree with the vector type.");

public:
    ChannelIterator(ChannelGetter<T, N>* parent, ind index) : Getter(parent), Index(index) {}

    ChannelIterator() : Getter(nullptr), Index(-1) {}

    ~ChannelIterator() { delete Getter; }

    ChannelIterator(const ChannelIterator<VecNT, T, N>& other)
        : Getter(other.Getter->New()), Index(other.Index) {}

    ChannelIterator<VecNT, T, N>& operator=(const ChannelIterator<VecNT, T, N>& other) {
        Getter = other->New();
        Index = other.Index;
        return *this;
    }

    /** Dereference to get data */
    VecNT& operator*();

    /*** Bidirectional Iteration ***\

    /** Walk forward */
    ChannelIterator<VecNT, T, N>& operator++() {
        Index++;
        return *this;
    }

    /** Walk backward */
    ChannelIterator<VecNT, T, N>& operator--() {
        Index--;
        return *this;
    }

    /** Compare */
    bool operator==(const ChannelIterator<VecNT, T, N>& other) const {
        return *other.Getter == *Getter && other.Index == Index;
    }

    /** Compare */
    bool operator!=(const ChannelIterator<VecNT, T, N>& other) const { return !(other == *this); }

    /*** Random Access Iteration ***/

    /** Increment randomly */
    ChannelIterator<VecNT, T, N> operator+(ind offset) {
        return ChannelIterator<VecNT, T, N>(Getter, Index + offset);
    }

    /** Increment randomly */
    ChannelIterator<VecNT, T, N> operator+=(ind offset) { Index += offset; }

    /** Decrement randomly */
    ChannelIterator<VecNT, T, N> operator-(ind offset) {
        return ChannelIterator<VecNT, T, N>(Getter, Index - offset);
    }

    /** Decrement randomly */
    ChannelIterator<VecNT, T, N> operator-=(ind offset) { Index -= offset; }

    // Members
protected:
    /** Abstract struct handling the dereferencing **/
    ChannelGetter<T, N>* Getter;

    /** Index to the current element */
    ind Index;
};

/** Increment randomly */
template <typename VecNT, typename T, ind N>
ChannelIterator<VecNT, T, N> operator+(ind offset, ChannelIterator<VecNT, T, N>& iter) {
    return ChannelIterator<VecNT, T, N>(iter.Getter, iter.Index + offset);
}

/** Decrement randomly */
template <typename VecNT, typename T, ind N>
ChannelIterator<VecNT, T, N> operator-(ind offset, ChannelIterator<VecNT, T, N>& iter) {
    return ChannelIterator<VecNT, T, N>(iter.Getter, iter.Index - offset);
}

/*********************************************************************************
 * Constant Iterator
 *********************************************************************************/

/** \struct ConstChannelIterator
 *   Generalized iterator over any const DataChannel.
 *   Returns by value using the DataChannel's fill.
 */
template <typename VecNT, typename T, ind N>
class ConstChannelIterator {

    static_assert(sizeof(VecNT) == sizeof(T) * N,
                  "Size and type do not agree with the vector type.");

public:
    ConstChannelIterator(const DataChannel<T, N>* parent, ind index)
        : Parent(parent), Index(index) {}

    ConstChannelIterator() : Parent(nullptr), Index(-1) {}

    /** Dereference to get data */
    VecNT operator*();

    /** Bidirectional Iteration **/

    /** Walk forward */
    ConstChannelIterator<VecNT, T, N>& operator++() {
        Index++;
        return *this;
    }

    /** Walk backward */
    ConstChannelIterator<VecNT, T, N>& operator--() {
        Index--;
        return *this;
    }

    /** Compare */
    bool operator==(ConstChannelIterator<VecNT, T, N>& other) {
        return other.Parent == Parent && other.Index == Index;
    }

    /** Compare */
    bool operator!=(ConstChannelIterator<VecNT, T, N>& other) { return !(other == *this); }

    /** Random Access Iteration **/

    /** Increment randomly */
    ConstChannelIterator<VecNT, T, N> operator+(ind offset) {
        return ConstChannelIterator<VecNT, T, N>(Parent, Index + offset);
    }

    /** Increment randomly */
    ConstChannelIterator<VecNT, T, N> operator+=(ind offset) { Index += offset; }

    /** Decrement randomly */
    ConstChannelIterator<VecNT, T, N> operator-(ind offset) {
        return ConstChannelIterator<VecNT, T, N>(Parent, Index - offset);
    }

    /** Decrement randomly */
    ConstChannelIterator<VecNT, T, N> operator-=(ind offset) { Index -= offset; }

    // Members
protected:
    /** Constant DataChannel */
    const DataChannel<T, N>* Parent;

    /** Index to the current element */
    ind Index;
};

/** Increment randomly */
template <typename VecNT, typename T, ind N>
ConstChannelIterator<VecNT, T, N> operator+(ind offset, ConstChannelIterator<VecNT, T, N>& iter) {
    return ConstChannelIterator<VecNT, T, N>(iter.Parent, iter.Index + offset);
}

/** Decrement randomly */
template <typename VecNT, typename T, ind N>
ConstChannelIterator<VecNT, T, N> operator-(ind offset, ConstChannelIterator<VecNT, T, N>& iter) {
    return ConstChannelIterator<VecNT, T, N>(iter.Parent, iter.Index - offset);
}

}  // namespace discretedata
}  // namespace inviwo

// Circumvent circular reference.
#include "datachannel.h"
#include "bufferchannel.h"

#include "channeliterator.inl"
