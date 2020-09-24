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

#include <ivwdiscretedata/pyconnectivities.h>

#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <warn/pop>

#include <modules/discretedata/discretedatatypes.h>
#include <modules/discretedata/connectivity/connectivity.h>
#include <modules/discretedata/connectivity/structuredgrid.h>

#include <modules/discretedatapython/channels/numpychannel.h>

#include <inviwo/core/util/defaultvalues.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <modules/python3/pyportutils.h>

#include <fmt/format.h>

using namespace pybind11::literals;
namespace py = pybind11;

namespace inviwo {

using namespace discretedata;

void exposeConnectivities(pybind11::module& m) {
    py::class_<Connectivity, std::shared_ptr<Connectivity>> grid(m, "Connectivity");
    grid.def("getDimension", &Connectivity::getDimension,
             "Get the dimension the grid is defined on.")
        .def("getNumElements", &Connectivity::getNumElements, "elementType"_a,
             "Get the number of elements of the specified dimension.")
        .def("createStructured", &discretepyutil::createStructuredGrid,  //"size"_a,
             "Create a new structured grid with the given number of vertices.");
}

std::shared_ptr<Connectivity> discretepyutil::createStructuredGrid(
    const pybind11::array_t<int>& size) {

    auto ndim = size.ndim();
    std::cout << "Shape:" << std::endl;
    for (ind n = 0; n < ndim; ++n) {
        std::cout << n << ": " << size.shape(n) << std::endl;
    }
    ivwAssert(ndim == 1, "Size array should be a vector.");
    ind numVerts = size.shape(0);
    // auto combinedFormat = inviwo::pyutil::getDataFormat(ndim == 1 ? 1 : size.shape(1), size);
    std::cout << "Dimension: " << numVerts << std::endl;

    detail::CreateStructuredGridDispatcher dispatcher;
    return channeldispatching::dispatchNumber<std::shared_ptr<Connectivity>, 1,
                                              DISCRETEDATA_MAX_NUM_DIMENSIONS>(numVerts, dispatcher,
                                                                               size);
    // return channeldispatching::dispatch<std::shared_ptr<DataSet>, dispatching::filter::Scalars,
    // 1,
    //                                     DISCRETEDATA_MAX_NUM_DIMENSIONS>(combinedFormat,
    //                                     dispatcher,
    //                                                                      size);
    // std::cout << "I'm fine." << combinedFormat << std::endl;
    // return nullptr;
}

}  // namespace inviwo
