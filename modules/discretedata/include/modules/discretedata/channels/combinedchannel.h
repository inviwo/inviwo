/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2020 Inviwo Foundation
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

#include <modules/discretedata/channels/datachannel.h>
#include <modules/discretedata/channels/channelgetter.h>
#include <modules/discretedata/channels/cachedgetter.h>

namespace inviwo {
namespace discretedata {

namespace detail {
template <typename T, ind ParentComponents>
struct FillRawDispatcher {
    template <typename Result, ind N>
    Result operator()(const Channel& ch, T* const dest, ind index, ind numElements) {
        const DataChannel<T, N>& chTN = dynamic_cast<const DataChannel<T, N>&>(ch);
        for (ind el = 0; el < numElements; ++el) {
            chTN.fillRaw(dest + el * ParentComponents, index + el, 1);
        }
    }
};
}  // namespace detail

/**
 * \brief Data channel combining several channels.
 *
 * Holds shared_ptr to several DataChannels of the same base type.
 */
template <typename T, ind N>
class CombinedChannel : public DataChannel<T, N> {

public:
    /**
     * \brief Direct construction
     * @param channels The channels to concatenate
     * @param name Name associated with the channel
     */
    CombinedChannel(const std::vector<std::shared_ptr<const Channel>>& channels,
                    const std::string& name)
        : DataChannel<T, N>(
              name, channels.size() ? channels[0]->getGridPrimitiveType() : GridPrimitive::Vertex)
        , channels_(channels) {
        if (channels_.size() < 2) {
            LogError("Not enough channels to combine.");
            channels_.clear();
            return;
        }

        auto firstScalarType = this->getDataFormatId();
        // const DataFormatBase* const firstScalarType = DataFormatBase::get(
        //     combinedFormat->getNumericType(), 1, combinedFormat->getPrecision());
        size_t numTotalComponents = 0;

        // Check all channels against the type of the first one.
        for (auto channel : channels_) {
            // combinedFormat = DataFormatBase::get(channel->getDataFormatId());
            auto scalarType = channel->getDataFormatId();
            //  DataFormatBase::get(combinedFormat->getNumericType(), 1,
            //                                       combinedFormat->getPrecision());

            if (scalarType != firstScalarType) {
                LogError(fmt::format(
                    "Scalar type of channel {} does not match first type encountered: {}.",
                    channel->getName(), DataFormatBase::get(firstScalarType)->getString()));
                channels_.clear();
            }
            numTotalComponents += channel->getNumComponents();
        }

        // Check the number of components.
        if (numTotalComponents != N) {
            LogError(
                fmt::format("Total number of components in channels ({}) does not match the size "
                            "provided to template ({}).",
                            numTotalComponents, N));
            channels_.clear();
        }
    }

    virtual ~CombinedChannel() = default;

    virtual Channel* clone() const override { return new CombinedChannel<T, N>(*this); }

    ind size() const override { return channels_[0]->size(); }

protected:
    /**
     * \brief Indexed point access, constant
     * Will write to the memory of dest via reinterpret_cast.
     * @param dest Position to write to, expect write of NumComponents many T
     * @param index Linear point index
     */
    void fillRaw(T* dest, ind index, ind numElements = 1) const override {
        T* copyDest = dest;
        detail::FillRawDispatcher<T, N> dispatcher;
        // for (ind el = 0; el < numElements; ++el) {
        for (auto channel : channels_) {  // std::tuple<T>
                                          // TODO: Make less sucky.
            // channel->dispatch<voi, dispatching::filter::Scalars, 1, N - 1>(
            channeldispatching::dispatchNumber<void, 1, N - 1>(
                channel->getNumComponents(), dispatcher, *channel, copyDest, index, numElements);
            // [&](auto ch) { ch->fillRaw(copyDest, index + el, 1); });
            // channel->fillRaw(reinterpret_cast<void*>(copyDest), index, 1);
            copyDest += channel->getNumComponents();
            // }
        }
    }

protected:
    virtual CachedGetter<CombinedChannel<T, N>>* newIterator() override {
        return new CachedGetter<CombinedChannel<T, N>>(this);
    }

public:
    std::vector<std::shared_ptr<const Channel>> channels_;
};

namespace detail {
struct CreateCombinedChannelHelper {
    template <typename Result, typename T, ind N>
    Result operator()(const std::vector<std::shared_ptr<const Channel>>& channels,
                      const std::string& name) {
        return std::make_shared<CombinedChannel<typename T::type, N>>(channels, name);
    }
};

}  // namespace detail

std::shared_ptr<Channel> createCombinedChannel(
    const std::vector<std::shared_ptr<const Channel>>& channels, const std::string& name);

}  // namespace discretedata
}  // namespace inviwo
