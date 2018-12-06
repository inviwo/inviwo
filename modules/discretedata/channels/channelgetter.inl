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

#include <modules/discretedata/channels/datachannel.h>
#include <modules/discretedata/channels/bufferchannel.h>

namespace inviwo {
namespace discretedata {

template <typename T, ind N>
T* BufferGetter<T, N>::get(ind index) {
    auto& buffer = dynamic_cast<BufferChannel<T, N>&>(*this->parent_);
    T& data = buffer.buffer_[index * N];
    return &data;
}

template <typename T, ind N>
const T* BufferGetter<T, N>::get(ind index) const {
    const BufferChannel<T, N>& buffer = dynamic_cast<const BufferChannel<T, N>&>(*this->parent_);
    const T& data = buffer.buffer_[index * N];
    return &data;
}

template <typename T, ind N>
ChannelGetter<T, N>* BufferGetter<T, N>::New() const {
    return new BufferGetter<T, N>(dynamic_cast<BufferChannel<T, N>*>(this->parent_));
}

template <typename T, ind N>
T* CachedGetter<T, N>::get(ind index) {
    assert(this->parent_ && "No channel to iterate is set.");

    // No buffer, evaluate analytics.
    // Is the data up to date?
    if (DataIndex != index) {
        delete Data;
        Data = new std::array<T, N>();
        // Note: we could use the old memory again, but numCOmponents might change (improbable).
        // Also, we want segfaults when old data is accessed.

        this->parent_->fill(*Data, index);
        DataIndex = index;
    }

    // Always return data.
    // If the iterator is changed and dereferenced, the pointer becomes invalid.
    return reinterpret_cast<T*>(Data);
}

template <typename T, ind N>
const T* CachedGetter<T, N>::get(ind index) const {
    assert(this->parent_ && "No channel to iterate is set.");

    // No buffer, evaluate analytics.
    // Is the data up to date?
    if (DataIndex != index) {
        delete Data;
        Data = new std::array<T, N>();
        // Note: we could use the old memory again, but numCOmponents might change (improbable).
        // Also, we want segfaults when old data is accessed.

        this->parent_->fill(*Data, index);
        DataIndex = index;
    }

    // Always return data.
    // If the iterator is changed and dereferenced, the pointer becomes invalid.
    return reinterpret_cast<T*>(Data);
}

}  // namespace discretedata
}  // namespace inviwo
