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

// namespace detail {
// template <typename T, ind ParentComponents>
// struct FillRawDispatcher {
//     template <typename Result, ind N>
//     Result operator()(const Channel& ch, T* const dest, ind index, ind numElements) {
//         const DataChannel<T, N>& chTN = dynamic_cast<const DataChannel<T, N>&>(ch);
//         for (ind el = 0; el < numElements; ++el) {
//             chTN.fillRaw(dest + el * ParentComponents, index + el, 1);
//         }
//     }
// };
// }  // namespace detail

/**
 * \brief Converts a channel into another base format
 *
 * Holds shared_ptr to a DataChannels and casts the values on access.
 */
template <typename T, typename FromT, ind N>
class FormatConversionChannel : public DataChannel<T, N> {

public:
    /**
     * \brief Direct construction
     * @param channels The channel to convert
     * @param name Name associated with the channel
     */
    FormatConversionChannel(const std::shared_ptr<const DataChannel<FromT, N>> channel,
                            const std::string& name)
        : DataChannel<T, N>(name, channel->getGridPrimitiveType()), channel_(channel) {
        this->setInvalidValue(static_cast<T>(channel->getInvalidValue()));
    }

    virtual ~FormatConversionChannel() = default;

    virtual Channel* clone() const override {
        return new FormatConversionChannel<T, FromT, N>(*this);
    }

    ind size() const override { return channel_->size(); }

protected:
    /**
     * \brief Indexed point access, constant
     * Will write to the memory of dest via reinterpret_cast.
     * @param dest Position to write to, expect write of NumComponents many T
     * @param index Linear point index
     */
    void fillRaw(T* dest, ind index, ind numElements = 1) const override {
        FromT* original = new FromT[numElements * N];
        channel_->fillRaw(original, index, numElements);
        for (ind i = 0; i < N * numElements; ++i) dest[i] = static_cast<T>(original[i]);
    }

protected:
    virtual CachedGetter<FormatConversionChannel<T, FromT, N>>* newIterator() override {
        return new CachedGetter<FormatConversionChannel<T, FromT, N>>(this);
    }

public:
    std::shared_ptr<const DataChannel<FromT, N>> channel_;
};

namespace dd_detail {
template <typename T, ind N>
struct CreateFormatConversionChannelToTN {
    template <typename Result, typename FromT>
    Result operator()(std::shared_ptr<const Channel> channel, const std::string& name) {
        auto dataChannel =
            std::dynamic_pointer_cast<const DataChannel<typename FromT::type, N>>(channel);

        if (!dataChannel) {
            std::cout << fmt::format("Input for conversion not of type <?, {}>", N) << std::endl;
            if (!channel) std::cout << "Well, was nullptr actually..." << std::endl;
            return nullptr;
        }
        auto convertedChannel =
            std::make_shared<FormatConversionChannel<T, typename FromT::type, N>>(dataChannel,
                                                                                  name);
        std::cout << fmt::format("Valid channel: {} (", convertedChannel ? "Yes" : "No");
        std::cout << convertedChannel->getName() << ")" << std::endl;

        auto ret = std::static_pointer_cast<DataChannel<T, N>>(convertedChannel);
        std::cout << fmt::format("Valid channel: {}", ret ? "Yes" : "No");
        return ret;
    }
};

struct CreateFormatConversionChannel {

    template <typename Result, typename T, ind N>
    Result operator()(std::shared_ptr<const Channel> channel, DataFormatId convertedFormat,
                      const std::string& name) {

        CreateFormatConversionChannelToTN<typename T::type, N> dispatcher;
        auto convertedChannel =
            dispatching::dispatch<std::shared_ptr<DataChannel<typename T::type, N>>,
                                  dispatching::filter::Scalars>(channel->getDataFormatId(),
                                                                dispatcher, channel, name);
        return std::static_pointer_cast<Channel>(convertedChannel);
        // return dispatching::dispatch<std::shared_ptr<Channel>, dispatching::filter::Scalars>(
        //     convertedFormat, dispatcher, channel, name);
        // detail::CreateFormatConversionChannelHelper dispatcher;
    }
};

// struct CreateFormatConversionChannelHelper {
//     template <typename FromT, ind N>
//     struct CreateChannel {
//         template <typename Result, typename T>
//         Result operator()(std::shared_ptr<const DataChannel<FromT::type, N>> channel,
//                           const std::string& name) {
//             return std::make_shared<FormatConversionChannel<T::type, FromT::type, N>>(channel,
//                                                                                       name);
//         }
//     };

//     template <typename Result, typename FromT, ind N>
//     Result operator()(std::shared_ptr<const Channel> channel, DataFormatId convertedFormat,
//                       const std::string& name) {
//         auto dataChannel = std::dynamic_pointer_cast<const DataChannel<FromT::type, N>>(channel);
//         if (!channel) return nullptr;

//         CreateChannel<FromT, N> dispatcher;
//         return dispatching::dispatch<std::shared_ptr<Channel>, dispatching::filter::Scalars>(
//             convertedFormat, dispatcher, channel, name);
//     }
// };

}  // namespace dd_detail

inline std::shared_ptr<Channel> createFormatConversionChannel(
    std::shared_ptr<const Channel> channel, DataFormatId convertedFormat, const std::string& name) {
    dd_detail::CreateFormatConversionChannel dispatcher;
    return channeldispatching::dispatch<std::shared_ptr<Channel>, dispatching::filter::Scalars, 1,
                                        DISCRETEDATA_MAX_NUM_DIMENSIONS>(
        convertedFormat, channel->getNumComponents(), dispatcher, channel, convertedFormat, name);

    // detail::CreateFormatConversionChannelHelper dispatcher;
    // return channeldispatching::dispatch<std::shared_ptr<Channel>, dispatching::filter::Scalars,
    // 1,
    //                                     DISCRETEDATA_MAX_NUM_DIMENSIONS>(
    //     channel->getDataFormatId(), channel->getNumComponents(), dispatcher, channel,
    //     convertedFormat, name);
}

}  // namespace discretedata
}  // namespace inviwo
