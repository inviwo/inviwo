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

#include <ivwdiscretedata/pydataset.h>

#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <warn/pop>

#include <modules/discretedata/discretedatatypes.h>
#include <modules/discretedata/dataset.h>
#include <modules/discretedata/connectivity/connectivity.h>
#include <modules/discretedata/ports/datasetport.h>

#include <inviwo/core/util/defaultvalues.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <modules/python3/pyportutils.h>

#include <fmt/format.h>

using namespace pybind11::literals;
namespace py = pybind11;

namespace inviwo {
using namespace discretedata;

void exposeDataSet(pybind11::module& m) {

    py::class_<DataSetInitializer>(m, "DataSetInitializer")
        .def_readwrite("name", &DataSetInitializer::name_)
        .def_readwrite("grid", &DataSetInitializer::grid_)
        .def_readwrite("channels", &DataSetInitializer::channels_);

    // std::shared_ptr<const Connectivity> (DataSet::*gridPtr)(void) = &DataSet::getGrid<>;
    py::class_<DataSet, std::shared_ptr<DataSet>> dataSet(m, "DataSet");
    dataSet
        .def(py::init<const std::string&, std::shared_ptr<const Connectivity>>(), "name"_a,
             "grid"_a, "Create an empty DataSet from a given grid.")
        .def(py::init<const DataSetInitializer&>(), "initializer"_a,
             "Create a DataSet from a given grid and channel list.")
        .def("addChannel",
             py::overload_cast<const std::shared_ptr<const Channel>&>(&DataSet::addChannel),
             // "channel"_a,
             "Add a new channel to the DataSet.")
        // .def("__repr__", &Processor::getIdentifier)
        // .def_static(
        //     "createFromArray", &discretepyutil::createStructuredDataSet, "vertData"_a, "name"_a,
        //     "Create a new DataSet with a StructuredGrid with positions from the array given.")
        ;
    ;
    // .def("getGrid", &DataSet::getGrid,
    //      //  static_cast<std::shared_ptr<const Connectivity>
    //      //  (DataSet::*)(void)>(&DataSet::getGrid),
    //      "Returns a shared pointer to the virtual grid.")
    //         .def("getChannel", py::overload_cast<int>(&DataSet::getChannel), "channel"_a,
    //         R"delim(
    // Add a new channel to the set.

    // Parameters
    // ----------
    // channel     Pointer to data
    // )delim")

    //         // interface for operator[]
    //         // .def(
    //         //     "__getitem__",
    //         //     [](std::shared_ptr<Channel> d, size_t i) {
    //         //         if (i >= d.size()()) throw py::index_error();
    //         //         return *(d.begin() + i);
    //         //     },
    //         //     py::return_value_policy::reference_internal)
    //         // // sequence protocol operations
    //         // .def(
    //         //     "__iter__", [](const DataFrame& d) { return py::make_iterator(d.begin(),
    //         d.end()); },
    //         //     py::keep_alive<0, 1>() /* Essential: keep object alive while iterator
    //         exists */)

    //         .def("__repr__", [](std::shared_ptr<Channel> channel) {
    //             return fmt::format("<Channel of type {} by {} with {} elements.>",
    //                                channel->getDataFromat(), channel->getNumComponents(),
    //                                channels->size());
    //         });

    // util::for_each_type<Scalars>{}(DataFrameAddColumnReg{}, dataframe);

    exposeStandardDataPorts<DataSet>(m, "DataSet");
    // exposeInport<DataInport<T>>(m, name);
    // exposeInport<DataInport<T, 0>>(m, name + "Multi");
    // exposeInport<DataInport<T, 0, true>>(m, name + "FlatMulti");
}

// std::shared_ptr<DataSet> discretepyutil::createStructuredDataSet(const pybind11::array& data,
//                                                                  const std::string& name) {

//     auto ndim = data.ndim();
//     std::cout << "Shape:" << std::endl;
//     for (ind n = 0; n < ndim; ++n) {
//         std::cout << n << ": " << data.shape(n) << std::endl;
//     }
//     ivwAssert(ndim == 1 || ndim == 2, "ndims must be either 1 or 2");
//     auto combinedFormat = inviwo::pyutil::getDataFormat(ndim == 1 ? 1 : data.shape(1), data);

//     detail::CreateStructuredDataSetDispatcher dispatcher;
//     return channeldispatching::dispatch<std::shared_ptr<DataSet>, dispatching::filter::Scalars,
//     1,
//                                         DISCRETEDATA_MAX_NUM_DIMENSIONS>(combinedFormat,
//                                         dispatcher,
//                                                                          data, name);
//     // std::cout << "I'm fine." << combinedFormat << std::endl;
//     // return nullptr;
// }

}  // namespace inviwo
