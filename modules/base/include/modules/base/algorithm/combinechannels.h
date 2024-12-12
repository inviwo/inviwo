/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2025 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/glmcomp.h>
#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/datastructures/nodata.h>
#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/datastructures/data.h>

#include <algorithm>
#include <span>
#include <ranges>
#include <vector>

namespace inviwo::util {

template <typename T, typename TRAMrep>
concept hasRAMrepresentation = requires(T data) {
    typename T::repr;
    requires std::is_base_of_v<typename T::repr, TRAMrep>;
    data.template getRepresentation<TRAMrep>();
};

namespace detail {

enum class ChannelNormalization { No, Yes };

}

/**
 * Combines up to four channels from four inports into a single object of type @p T. The data format
 * of the resulting container is based on the common numeric type and precision of the input
 * channels.
 *
 * @tparam T        underlying datatype
 * @tparam TRAMrep  RAM representation for @p T
 * @tparam channelNormalization  normalize each channel using its corresponding data range and then
 * renormalize it to the common data format
 * @param sources   array of inports for type @p T
 * @param selectedPortChannels  selected channel for each inport
 * @return instance of T with the combined channels of @p sources
 *
 * Usage:  <tt>combineChannels<Layer, LayerRAM>(...)</tt>
 * @see LayerCombiner, VolumeChannelCombiner
 */
template <typename T, typename TRAMrep,
          detail::ChannelNormalization normalization = detail::ChannelNormalization::No>
std::shared_ptr<T> combineChannels(const std::array<DataInport<T>, 4>& sources,
                                   const std::array<int, 4>& selectedPortChannels)
    requires hasRAMrepresentation<T, TRAMrep>
{
    using PortChannel = std::pair<const DataInport<T>*, int>;

    const auto activePorts = [&]() {
        std::vector<PortChannel> res;
        for (auto&& [port, channel] : util::zip(sources, selectedPortChannels)) {
            if (port.hasData()) {
                res.emplace_back(&port, channel);
            }
        }
        return res;
    }();

    if (auto it =
            std::ranges::adjacent_find(activePorts, std::not_equal_to<>{},
                                       [](auto& p) { return p.first->getData()->getDimensions(); });
        it != activePorts.end()) {
        throw Exception(SourceContext{},
                        "Dimensions of all inports need to be identical, found {} and {}",
                        it->first->getData()->getDimensions(),
                        std::next(it)->first->getData()->getDimensions());
    }

    auto&& [type, precision] = [&]() {
        std::vector<const DataFormatBase*> formats;
        for (auto p : activePorts) {
            formats.push_back(p.first->getData()->getDataFormat());
        }
        return std::make_pair(util::commonNumericType(formats),
                              util::commonFormatPrecision(formats));
    }();

    auto commonFormat = DataFormatBase::get(type, activePorts.size(), precision);

    using Config = typename T::Config;
    auto data =
        std::make_shared<T>(*activePorts.front().first->getData(), noData,
                            Config{.format = commonFormat,
                                   .swizzleMask = swizzlemasks::defaultData(activePorts.size())});

    [[maybe_unused]] DataMapper destDataMap(commonFormat);

#include <warn/push>
#include <warn/ignore/conversion>
#include <warn/ignore/conversion-loss>

    data->template getEditableRepresentation<TRAMrep>()->template dispatch<void>(
        [&activePorts, destDataMap]<typename DestRAMrep>(DestRAMrep* ramrep) {
            using PrecisionType = util::PrecisionValueType<DestRAMrep>;
            using ValueType = util::value_type_t<PrecisionType>;

            const auto dims{ramrep->getDimensions()};
            auto destData = ramrep->getDataTyped();

            for (size_t inputChannel = 0; inputChannel < activePorts.size(); ++inputChannel) {
                auto&& [port, srcChannel] = activePorts[inputChannel];

                if constexpr (normalization == detail::ChannelNormalization::Yes) {
                    port->getData()->template getRepresentation<TRAMrep>()->template dispatch<void>(
                        [dims, destData, inputChannel, srcDataMap = port->getData()->dataMap,
                         destDataMap](auto* srcramrep, int srcChannelArg) {
                            const auto srcData = srcramrep->getDataTyped();

                            for (size_t i = 0; i < glm::compMul(dims); ++i) {
                                auto normalizedValue = srcDataMap.mapFromDataToNormalized(
                                    util::glmcomp(srcData[i], srcChannelArg));
                                util::glmcomp(destData[i], inputChannel) = static_cast<ValueType>(
                                    destDataMap.mapFromNormalizedToData(normalizedValue));
                            }
                        },
                        srcChannel);
                } else {
                    port->getData()->template getRepresentation<TRAMrep>()->template dispatch<void>(
                        [dims, destData, inputChannel](auto* srcramrep, int srcChannelArg) {
                            const auto srcData = srcramrep->getDataTyped();

                            for (size_t i = 0; i < glm::compMul(dims); ++i) {
                                util::glmcomp(destData[i], inputChannel) = static_cast<ValueType>(
                                    util::glmcomp(srcData[i], srcChannelArg));
                            }
                        },
                        srcChannel);
                }
            }
        });

#include <warn/pop>
    return data;
}

}  // namespace inviwo::util
