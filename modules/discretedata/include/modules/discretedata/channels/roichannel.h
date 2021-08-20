/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2021 Inviwo Foundation
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

#include <modules/discretedata/channels/datachannel.h>

namespace inviwo {
namespace discretedata {

/**
 * \brief Subrange of a channel
 */
template <typename T, ind N = 1>
class ROIChannel : public DataChannel<T, N> {

public:
    /**
     * \brief Direct construction, empty data
     * @param baseChannel Original channel
     * @param offset First index to take
     * @param size Number of data points to take
     */
    ROIChannel(std::shared_ptr<const DataChannel<T, N>>& baseChannel, size_t offset, size_t size)
        : DataChannel<T, N>(fmt::format("{}_ROI", baseChannel->getName()))
        , baseChannel_(baseChannel)
        , offset_(offset)
        , size_(size) {}

    virtual Channel* clone() const override { return new ROIChannel<T, N>(*this); }

    virtual ind size() const override { return size_; }

    virtual CachedGetter<ROIChannel>* newIterator() override {
        return new CachedGetter<ROIChannel>(this);
    }

protected:
    /**
     * \brief Indexed point access, apply offset
     * @param dest Position to write to, expect write of NumComponents many T
     * @param index Linear point index
     */
    virtual void fillRaw(T* dest, ind index, ind numElements) const override {
        baseChannel_->fillRaw(dest, index - offset_, numElements);
    }

    const std::shared_ptr<const DataChannel<T, N>> baseChannel_;
    const ind offset_, size_;
};

struct CreateROIChannel {

    template <typename Result, typename T, ind N, typename... Args>
    Result operator()(const std::shared_ptr<const Channel>& channel, size_t offset, size_t size) {
        auto dataChannel =
            std::dynamic_pointer_cast<const DataChannel<typename T::type, N>>(channel);
        if (!dataChannel) {
            return nullptr;
        }

        return std::make_shared<ROIChannel<typename T::type, N>>(dataChannel, offset, size);
    }
};

std::shared_ptr<const Channel> createROIChannel(const std::shared_ptr<const Channel>& channel,
                                                size_t offset, size_t size) {
    CreateROIChannel dispatcher;
    return channeldispatching::dispatch<std::shared_ptr<Channel>, dispatching::filter::All, 1,
                                        DISCRETEDATA_MAX_NUM_DIMENSIONS>(
        channel->getDataFormatId(), channel->getNumComponents(), dispatcher, channel, offset, size);
}

// template <typename ChannelType>
// ROIChannel(std::shared_ptr<ChannelType>&) -> ROIChannel<ChannelType::T, ChannelType::N>;

}  // namespace discretedata
}  // namespace inviwo
