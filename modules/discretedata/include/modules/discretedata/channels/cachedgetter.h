/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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
#include <modules/discretedata/channels/datachannel.h>

namespace inviwo {
namespace discretedata {

template <typename Parent>
struct CachedGetter : public ChannelGetter<typename Parent::value_type, Parent::num_comp> {
    using value_type = typename Parent::value_type;
    static constexpr int num_comp = Parent::num_comp;

    CachedGetter(Parent* parent)
        : ChannelGetter<value_type, num_comp>(), dataIndex(-1), parent_{parent} {}
    virtual ~CachedGetter() = default;
    virtual CachedGetter* clone() const override { return new CachedGetter(parent_); }

    virtual value_type* get(ind index) override {
        assert(this->parent_ && "No channel to iterate is set.");

        // Is the data up to date?
        if (dataIndex != index) {
            this->parent_->fill(data, index);
            dataIndex = index;
        }

        // Always return data.
        // If the iterator is changed and dereferenced, the pointer becomes invalid.
        return data.data();
    }

protected:
    virtual Channel* parent() const override { return parent_; }

    //! Memory is invalidated on iteration
    std::array<value_type, num_comp> data;

    //! Index that is currently pointed to
    ind dataIndex;

    Parent* parent_;
};

}  // namespace discretedata
}  // namespace inviwo
