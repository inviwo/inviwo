/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

namespace inviwo {

namespace util {

/**
 * Combines up to four channels from four inports into a single object of type @p T.
 *
 * @tparam T        underlying datatype
 * @tparam TRAMrep  RAM representation for @p T
 * @param sources   array of inports for type @p T
 * @param selectedPortChannels  selected channel for each inport
 * @return instance of T with the combined channels of @p sources
 *
 * Usage:  <tt>combineChannels<Layer, LayerRAM>(...)</tt>
 * @see LayerCombiner, VolumeChannelCombiner
 */
template <typename T, typename TRAMrep>
std::shared_ptr<T> combineChannels(const std::array<DataInport<T>, 4>& sources,
                                   const std::array<int, 4>& selectedPortChannels)
    requires requires(T data) {
        typename T::repr;
        std::is_base_of_v<typename T::repr, TRAMrep>;
        data.getRepresentation<TRAMrep>();
    }
{

    using PortChannel = std::pair<const DataInport<T>*, int>;
    std::vector<PortChannel> activePorts;
    for (auto&& [port, channel] : util::zip(sources, selectedPortChannels)) {
        if (port.hasData()) {
            activePorts.emplace_back(&port, channel);
        }
    }

    const auto dims{activePorts.front().first->getData()->getDimensions()};
    if (std::ranges::any_of(
            activePorts, [dims](auto& p) { return p.first->getData()->getDimensions() != dims; })) {
        throw Exception("Dimensions of all inports need to be identical",
                        IVW_CONTEXT_CUSTOM("util::combineChannels"));
    }

    auto&& [type, precision] = [&]() {
        std::vector<const DataFormatBase*> formats;
        for (auto p : activePorts) {
            formats.push_back(p.first->getData()->getDataFormat());
        }
        return std::make_pair(util::commonNumericType(formats),
                              util::commonFormatPrecision(formats));
    }();

    auto data = std::make_shared<T>(*activePorts.front().first->getData(), noData, std::nullopt,
                                    DataFormatBase::get(type, activePorts.size(), precision));

#include <warn/push>
#include <warn/ignore/conversion>
#include <warn/ignore/conversion-loss>

    data->getEditableRepresentation<TRAMrep>()->dispatch<void>([&](auto ramrep) {
        using PrecisionType = util::PrecisionValueType<decltype(ramrep)>;
        using ValueType = util::value_type_t<PrecisionType>;

        const auto dims{ramrep->getDimensions()};
        auto destData = ramrep->getDataTyped();

        for (size_t inputChannel = 0; inputChannel < activePorts.size(); ++inputChannel) {
            auto&& [port, srcChannel] = activePorts[inputChannel];
            port->getData()->getRepresentation<TRAMrep>()->dispatch<void>(
                [&](auto srcramrep, int srcChannelArg) {
                    const auto srcData = srcramrep->getDataTyped();

                    for (size_t i = 0; i < glm::compMul(dims); ++i) {
                        util::glmcomp(destData[i], inputChannel) =
                            static_cast<ValueType>(util::glmcomp(srcData[i], srcChannelArg));
                    }
                },
                srcChannel);
        }
    });

#include <warn/pop>

    data->setSwizzleMask(swizzlemasks::defaultData(activePorts.size()));
    return data;
}

}  // namespace util

}  // namespace inviwo
