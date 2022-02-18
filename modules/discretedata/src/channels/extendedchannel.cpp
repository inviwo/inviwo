/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2020 Inviwo Foundation
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

#include <modules/discretedata/channels/extendedchannel.h>
#include <modules/discretedata/channels/channeldispatching.h>
#include <inviwo/core/util/assertion.h>

namespace inviwo {
namespace discretedata {

std::shared_ptr<Channel> createExtendedChannel(const std::shared_ptr<const Channel>& baseChannel,
                                               const std::shared_ptr<const Channel>& extendChannel,
                                               const std::string& name) {

    if (!baseChannel || !extendChannel) return nullptr;

    auto firstScalarId = baseChannel->getDataFormatId();
    size_t numTotalComponents = baseChannel->getNumComponents() + extendChannel->getNumComponents();

    detail::CreateExtendedChannelHelper dispatcher;
    return channeldispatching::dispatch<std::shared_ptr<Channel>, dispatching::filter::Scalars, 1,
                                        DISCRETEDATA_MAX_NUM_DIMENSIONS>(
        firstScalarId, numTotalComponents, dispatcher, baseChannel, extendChannel, name);
}
}  // namespace discretedata

}  // namespace inviwo
