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

#include <modules/discretedata/discretedatatypes.h>

namespace inviwo {
namespace discretedata {

class Channel;

template <typename T, ind N>
struct ChannelGetter {
    ChannelGetter() = default;
    ChannelGetter(const ChannelGetter&) = delete;
    ChannelGetter(ChannelGetter&&) = delete;
    ChannelGetter& operator=(const ChannelGetter&) = delete;
    ChannelGetter& operator=(ChannelGetter&&) = delete;
    virtual ~ChannelGetter() = default;

    virtual T* get(ind index) = 0;

    //! Compare by the parent pointer
    bool operator==(const ChannelGetter<T, N>& other) const { return equal(other); }
    bool operator!=(const ChannelGetter<T, N>& other) const { return !(*this == other); }
    virtual ChannelGetter<T, N>* clone() const = 0;

protected:
    virtual bool equal(const ChannelGetter<T, N>& other) const {
        return parent() == other.parent();
    }
    virtual Channel* parent() const = 0;
};

}  // namespace discretedata
}  // namespace inviwo
