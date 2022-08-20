/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
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
#include <inviwo/core/datastructures/datatraits.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/demangle.h>

#include <fmt/format.h>

namespace inviwo {

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
pybind11::class_<util::IterRangeGenerator<Iter>> exposeIterRangeGenerator(pybind11::handle& m,
                                                                          const std::string& name) {

    return pybind11::class_<util::IterRangeGenerator<Iter>>(m,
                                                            StrBuffer{"{}Generator", name}.c_str())
        .def("__next__", &util::IterRangeGenerator<Iter>::next);
}

template <typename Port>
pybind11::class_<Port, Outport> exposeOutport(pybind11::module& m, const std::string& name) {
    namespace py = pybind11;
    using T = typename Port::type;

    return pybind11::class_<Port, Outport>(m, StrBuffer{"{}Outport", name}.c_str())
        .def(py::init<std::string>())
        .def("getData", &Port::getData)
        .def("detatchData", &Port::detachData)
        .def("setData", [](Port* p, std::shared_ptr<const T> data) { p->setData(data); });
}

template <typename Port>
pybind11::class_<Port, Inport> exposeInport(pybind11::module& m, const std::string& name) {

    namespace py = pybind11;

    pybind11::class_<Port, Inport> pyInport{m, StrBuffer{"{}Inport", name}.c_str()};

    exposeIterRangeGenerator<typename Port::const_iterator>(pyInport, "Data");
    exposeIterRangeGenerator<typename Port::const_iterator_port>(pyInport, "OutportAndData");
    exposeIterRangeGenerator<typename Port::const_iterator_changed>(pyInport, "ChangedAndData");

    pyInport.def(py::init<std::string>())
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

    return pyInport;
}

template <typename T>
void exposeStandardDataPorts(pybind11::module& m, const std::string& name) {
    if (DataTraits<T>::classIdentifier().empty()) {
        throw Exception(
            fmt::format("exposing standard DataPorts to python for '{0}' failed due to missing "
                        "class identifier. Have you provided a DataTraits<{0}> specialization?",
                        util::parseTypeIdName(typeid(T).name())),
            IVW_CONTEXT_CUSTOM("exposeStandardDataPorts"));
    }

    exposeInport<DataInport<T>>(m, name);
    exposeInport<DataInport<T, 0>>(m, name + "Multi");
    exposeInport<DataInport<T, 0, true>>(m, name + "FlatMulti");
    exposeOutport<DataOutport<T>>(m, name);
}

}  // namespace inviwo
