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

#include <ivwdiscretedata/pychannels.h>

#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <warn/pop>

#include <modules/discretedata/discretedatatypes.h>
#include <modules/discretedata/dataset.h>
#include <modules/discretedata/channels/channel.h>
#include <modules/discretedata/channels/bufferchannel.h>
#include <modules/discretedata/channels/datachannel.h>

#include <modules/discretedatapython/channels/numpychannel.h>

#include <inviwo/core/util/defaultvalues.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <modules/python3/pyportutils.h>

#include <fmt/format.h>

using namespace pybind11::literals;
namespace py = pybind11;

namespace inviwo {
using namespace discretedata;

void exposeChannels(pybind11::module& m) {

    py::enum_<GridPrimitive>(m, "GridPrimitive")
        .value("Undef", GridPrimitive::Undef)
        .value("Vertex", GridPrimitive::Vertex)
        .value("Edge", GridPrimitive::Edge)
        .value("Face", GridPrimitive::Face)
        .value("Volume", GridPrimitive::Volume)
        .value("HyperVolume", GridPrimitive::HyperVolume)
        .export_values();

    py::class_<Channel, std::shared_ptr<Channel>> channel(m, "Channel");
    channel.def("clone", &Channel::clone, "Get a deep copy of the underlying channel.")
        .def("getName", &Channel::getName, "Get the channel name.")
        .def("setName", &Channel::setName, "name"_a, "Set the channel name.")
        .def("getGridPrimitiveType", &Channel::getGridPrimitiveType,
             "Get the type of element this data is defined on, e.g. vertex or face.")
        .def(
            "getDataFormatId",
            [](const Channel& c) {
                return inviwo::pyutil::toNumPyFormat(DataFormatBase::get(c.getDataFormatId()));
            },
            "Get the scalar type of the data, e.g. Float64 for vec3 data.")
        .def("getNumComponents", &Channel::getNumComponents,
             "The number of vector components, e.g. 3 for vec3 data.")
        .def("size", &Channel::size, "Number of elements, e.g. number of vec3s.")
        .def_static("createChannel", &discretepyutil::createPyChannel,
                    //  "dataArray"_a, "name"_a, "definedOnPrimitive"_a,
                    "Create a new NumPyChannel from the array given.");
    //  const pybind11::array& data, const std::string& name,
    //    GridPrimitive definedOn = GridPrimitive::Vertex

    //////////////// TEMPLATE THIS!!!!
    // py::class_<NumPyChannel, std::shared_ptr<NumPyChannel>> numPyChannel(m, "NumPyChannel");
    // channel
    //     .def("clone", &NumPyChannel::clone, "Get a deep copy of the underlying channel.")
    //     // interface for operator[]
    //     .def(
    //         "__getitem__", [](std::shared_ptr<NumPyChannel> c, size_t i) { return (*c)[i]; },
    //         py::return_value_policy::reference_internal)
    //     .def(py::init<std::shared_ptr<pybind11::array>>(), "data"_a,
    //          "Create a new NumPyChannel from the array given.");
}

std::shared_ptr<Channel> discretepyutil::createPyChannel(const pybind11::array& data,
                                                         const std::string& name,
                                                         GridPrimitive definedOn) {

    auto ndim = data.ndim();
    // ivwAssert(ndim == 1 || ndim == 2, "ndims must be either 1 or 2");
    std::cout << "Got data type " << data.dtype().kind() << std::endl;
    // auto combinedFormat = inviwo::pyutil::getDataFormat(1, data);
    auto combinedFormat = DataFormatBase::get(DataFormatId::Float32);

    detail::CreatePyChannelHelper dispatcher;
    return channeldispatching::dispatch<std::shared_ptr<Channel>, dispatching::filter::Scalars, 1,
                                        DISCRETEDATA_MAX_NUM_DIMENSIONS>(combinedFormat, dispatcher,
                                                                         data, name, definedOn);
}

std::shared_ptr<Channel> discretepyutil::createBufferChannel(const pybind11::array& data,
                                                             const std::string& name,
                                                             GridPrimitive definedOn) {

    auto ndim = data.ndim();
    ivwAssert(ndim == 1 || ndim == 2, "ndims must be either 1 or 2");
    auto combinedFormat = inviwo::pyutil::getDataFormat(ndim == 1 ? 1 : data.shape(1), data);

    detail::CreateBufferChannelHelper dispatcher;
    return channeldispatching::dispatch<std::shared_ptr<Channel>, dispatching::filter::All, 1, 7>(
        combinedFormat, dispatcher, data, name, definedOn);
}

}  // namespace inviwo
