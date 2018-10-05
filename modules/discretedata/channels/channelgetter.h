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

namespace inviwo {
namespace discretedata {

template <typename T, ind N>
class BufferChannel;

template <typename T, ind N>
class DataChannel;

template <typename T, ind N>
struct ChannelGetter {

    ChannelGetter(DataChannel<T, N>* parent) : parent_(parent) {}

    /** Dereference to get data */
    virtual T* get(ind index) = 0;

    /** Dereference to get data */
    virtual const T* get(ind index) const = 0;

    /** Compare by the parent pointer */
    bool operator==(const ChannelGetter<T, N>& other) const {
        return this->parent_ == other.parent_;
    }
    bool operator!=(const ChannelGetter<T, N>& other) const { return !(*this == other); }
    virtual ChannelGetter<T, N>* New() const = 0;

protected:
    /** Pointer to DataChannel iterated through - Do not delete */
    DataChannel<T, N>* parent_;
};

template <typename T, ind N>
struct BufferGetter : public ChannelGetter<T, N> {

    BufferGetter(BufferChannel<T, N>* parent) : ChannelGetter<T, N>(parent) {}

    /** Dereference to get data */
    virtual T* get(ind index);

    virtual const T* get(ind index) const;

    virtual ChannelGetter<T, N>* New() const override;
};

template <typename T, ind N>
struct CachedGetter : public ChannelGetter<T, N> {

    CachedGetter(DataChannel<T, N>* parent)
        : ChannelGetter<T, N>(parent), Data(nullptr), DataIndex(-1) {}

    /** Dereference to get data */
    virtual T* get(ind index) override;

    virtual const T* get(ind index) const override;

    virtual ChannelGetter<T, N>* New() const override {
        return new CachedGetter<T, N>(this->parent_);
    }

    /** Pointer to heap data
     * - Memory is invalidated on iteration */
    mutable std::array<T, N>* Data;

    /** Index that is currently pointed to */
    mutable ind DataIndex;
};

}  // namespace discretedata
}  // namespace inviwo

#include "channelgetter.inl"
