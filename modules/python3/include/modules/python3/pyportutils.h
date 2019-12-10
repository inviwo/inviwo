/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <warn/pop>

#include <inviwo/core/ports/port.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

namespace detail {
template <typename T>
struct PortDeleter {
    void operator()(T* p) {
        if (p && p->getProcessor() == nullptr) delete p;
    }
};
}  // namespace detail

template <typename T>
using PortPtr = std::unique_ptr<T, detail::PortDeleter<T>>;

namespace util {

template <typename Iter>
struct IterRangeGenerator : iter_range<Iter> {
    IterRangeGenerator(iter_range<Iter> base) : iter_range<Iter>{base} {};
    using iter_range<Iter>::iter_range;
    typename Iter::value_type next() {
        if (this->first != this->second) {
            return *(this->first++);
        } else {
            throw pybind11::stop_iteration{};
        }
    }
};

}  // namespace util

template <typename Iter>
pybind11::class_<util::IterRangeGenerator<Iter>> exposeIterRangeGenerator(pybind11::module& m,
                                                                          const std::string& name) {

    return pybind11::class_<util::IterRangeGenerator<Iter>>(m, (name + "Generator").c_str())
        .def("__next__", &util::IterRangeGenerator<Iter>::next);
}

template <typename Port>
pybind11::class_<Port, Outport, PortPtr<Port>> exposeOutport(pybind11::module& m,
                                                             const std::string& name) {
    namespace py = pybind11;
    using T = typename Port::type;
    return pybind11::class_<Port, Outport, PortPtr<Port>>(m, (name + "Outport").c_str())
        .def(py::init<std::string>())
        .def("getData", &Port::getData)
        .def("detatchData", &Port::detachData)
        .def("setData", [](Port* port, std::shared_ptr<T> data) { port->setData(data); });
}

template <typename Port>
pybind11::class_<Port, Inport, PortPtr<Port>> exposeInport(pybind11::module& m,
                                                           const std::string& name) {

    exposeIterRangeGenerator<typename Port::const_iterator>(m, name + "InportData");
    exposeIterRangeGenerator<typename Port::const_iterator_port>(m, name + "InportOutportAndData");
    exposeIterRangeGenerator<typename Port::const_iterator_changed>(m,
                                                                    name + "InportChangedAndData");

    namespace py = pybind11;
    return pybind11::class_<Port, Inport, PortPtr<Port>>(m, (name + "Inport").c_str())
        .def(py::init<std::string>())
        .def("hasData", &Port::hasData)
        .def("getData", &Port::getData)
        .def("getVectorData", &Port::getVectorData)
        .def("getSourceVectorData", &Port::getSourceVectorData)
        .def("data",
             [](Port* p) {
                 return util::IterRangeGenerator<typename Port::const_iterator>(p->begin(),
                                                                                p->end());
             })
        .def("outportAndData",
             [](Port* p) {
                 return util::IterRangeGenerator<typename Port::const_iterator_port>(
                     p->outportAndData());
             })
        .def("changedAndData", [](Port* p) {
            return util::IterRangeGenerator<typename Port::const_iterator_changed>(
                p->changedAndData());
        });
}

template <typename T>
void exposeStandardDataPorts(pybind11::module& m, const std::string& name) {
    exposeOutport<DataOutport<T>>(m, name);
    exposeInport<DataInport<T>>(m, name);
    exposeInport<DataInport<T, 0>>(m, name + "Multi");
    exposeInport<DataInport<T, 0, true>>(m, name + "FlatMulti");
}

}  // namespace inviwo
