/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2020 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/pybind11.h>
#include <warn/pop>
#include <modules/discretedatapython/channels/numpychannel.h>
#include <modules/discretedata/channels/channeldispatching.h>

namespace inviwo {
using namespace discretedata;

void exposeChannels(pybind11::module& m);

namespace discretepyutil {
namespace detail {

struct CreatePyChannelHelper {
    template <typename Result, typename T, ind N>
    Result operator()(const pybind11::array& data, const std::string& name,
                      GridPrimitive definedOn) {
        return std::make_shared<NumPyChannel<typename T::type, N>>(data, name, definedOn);
    }
};

struct CreateBufferChannelHelper {

    template <typename Result, typename T, ind N>
    Result operator()(const pybind11::array& data, const std::string& name,
                      GridPrimitive definedOn) {
        auto channel =
            std::make_shared<BufferChannel<typename T::type, N>>(data.size(), name, definedOn);
        memcpy(reinterpret_cast<void*>(channel->data().data()),
               reinterpret_cast<const void*>(data.data(0)), data.nbytes());
        channel->setInvalidValueDouble(42.0);
        return channel;
    }
};
}  // namespace detail

std::shared_ptr<Channel> createPyChannel(const pybind11::array& data, const std::string& name,
                                         GridPrimitive definedOn = GridPrimitive::Vertex);

std::shared_ptr<Channel> createBufferChannel(const pybind11::array& data, const std::string& name,
                                             GridPrimitive definedOn = GridPrimitive::Vertex);

}  // namespace discretepyutil
}  // namespace inviwo
