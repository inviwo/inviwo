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
#include <modules/discretedata/channels/channelgetter.h>

namespace inviwo {
namespace discretedata {

/** \struct ChannelIterator
 *   Generalized iterator over any DataChannel.
 *   Handles data deletion if necessary - runtime check whether needed.
 */
template <typename VecNT, typename T, ind N>
class ChannelIterator {
    static_assert(sizeof(VecNT) == sizeof(T) * N,
                  "Size and type do not agree with the vector type.");

public:
    ChannelIterator(ChannelGetter<T, N>* parent, ind index) : getter(parent), index(index) {}
    ChannelIterator() : getter(nullptr), index(-1) {}
    ~ChannelIterator() = default;
    ChannelIterator(const ChannelIterator<VecNT, T, N>& other)
        : getter(other.getter->clone()), index(other.index) {}
    ChannelIterator<VecNT, T, N>& operator=(const ChannelIterator<VecNT, T, N>& other) {
        getter.reset(other->clone());
        index = other.index;
        return *this;
    }

    VecNT& operator*();

    // Bidirectional iterator
    ChannelIterator<VecNT, T, N>& operator++() {
        index++;
        return *this;
    }
    ChannelIterator<VecNT, T, N> operator++(int) {
        auto i = *this;
        index++;
        return i;
    }
    ChannelIterator<VecNT, T, N>& operator--() {
        index--;
        return *this;
    }
    ChannelIterator<VecNT, T, N>& operator--(int) {
        auto i = *this;
        index--;
        return i;
    }

    // Random Access iterator
    ChannelIterator<VecNT, T, N> operator+(ind offset) {
        return ChannelIterator<VecNT, T, N>(getter, index + offset);
    }
    ChannelIterator<VecNT, T, N> operator+=(ind offset) { index += offset; }
    ChannelIterator<VecNT, T, N> operator-(ind offset) {
        return ChannelIterator<VecNT, T, N>(getter, index - offset);
    }
    ChannelIterator<VecNT, T, N> operator-=(ind offset) { index -= offset; }

    // compare
    bool operator==(const ChannelIterator<VecNT, T, N>& other) const {
        return *other.getter == *getter && other.index == index;
    }
    bool operator!=(const ChannelIterator<VecNT, T, N>& other) const { return !(other == *this); }

protected:
    /** Abstract struct handling the dereferencing **/
    std::unique_ptr<ChannelGetter<T, N>> getter;

    /** Index to the current element */
    ind index;
};

/** Increment randomly */
template <typename VecNT, typename T, ind N>
ChannelIterator<VecNT, T, N> operator+(ind offset, ChannelIterator<VecNT, T, N>& iter) {
    return ChannelIterator<VecNT, T, N>(iter.getter, iter.index + offset);
}

/** Decrement randomly */
template <typename VecNT, typename T, ind N>
ChannelIterator<VecNT, T, N> operator-(ind offset, ChannelIterator<VecNT, T, N>& iter) {
    return ChannelIterator<VecNT, T, N>(iter.getter, iter.index - offset);
}

/*********************************************************************************
 * Constant Iterator
 *********************************************************************************/

/** \struct ConstChannelIterator
 *   Generalized iterator over any const DataChannel.
 *   Returns by value using the DataChannel's fill.
 */
template <typename Parent, typename VecNT>
class ConstChannelIterator {
public:
    using T = typename Parent::value_type;
    static constexpr ind num_comp = Parent::num_comp;
    static_assert(sizeof(VecNT) == sizeof(T) * num_comp,
                  "Size and type do not agree with the vector type.");

    ConstChannelIterator(const Parent* parent, ind index) : parent(parent), index(index) {}
    ConstChannelIterator() : parent(nullptr), index(-1) {}

    VecNT operator*();

    // Bidirectional iterator
    ConstChannelIterator& operator++() {
        index++;
        return *this;
    }
    ConstChannelIterator operator++(int) {
        auto i = *this;
        index++;
        return i;
    }
    ConstChannelIterator& operator--() {
        index--;
        return *this;
    }
    ConstChannelIterator& operator--(int) {
        auto i = *this;
        index--;
        return *this;
    }

    // Random Access iterator
    ConstChannelIterator operator+(ind offset) {
        return ConstChannelIterator(parent, index + offset);
    }
    ConstChannelIterator operator+=(ind offset) { index += offset; }
    ConstChannelIterator operator-(ind offset) {
        return ConstChannelIterator(parent, index - offset);
    }
    ConstChannelIterator operator-=(ind offset) { index -= offset; }

    // compare
    bool operator==(ConstChannelIterator& other) {
        return other.parent == parent && other.index == index;
    }
    bool operator!=(ConstChannelIterator& other) { return !(other == *this); }

protected:
    //! Constant DataChannel
    const Parent* parent;

    //! index to the current element
    ind index;
};

//! Increment randomly
template <typename Parent, typename VecNT>
ConstChannelIterator<Parent, VecNT> operator+(ind offset,
                                              ConstChannelIterator<Parent, VecNT>& iter) {
    return ConstChannelIterator<Parent, VecNT>(iter.parent, iter.index + offset);
}

//! Decrement randomly
template <typename Parent, typename VecNT>
ConstChannelIterator<Parent, VecNT> operator-(ind offset,
                                              ConstChannelIterator<Parent, VecNT>& iter) {
    return ConstChannelIterator<Parent, VecNT>(iter.parent, iter.index - offset);
}

template <typename Parent, typename VecNT>
VecNT ConstChannelIterator<Parent, VecNT>::operator*() {
    VecNT data;
    parent->fill(data, index);
    return data;
}

template <typename VecNT, typename T, inviwo::discretedata::ind N>
VecNT& ChannelIterator<VecNT, T, N>::operator*() {
    T* data = getter->get(index);
    return *reinterpret_cast<VecNT*>(data);
}

}  // namespace discretedata
}  // namespace inviwo
