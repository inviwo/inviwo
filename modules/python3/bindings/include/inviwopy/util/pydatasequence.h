/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <pybind11/operators.h>
#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/numpy.h>

#include <inviwo/core/datastructures/datasequence.h>

#include <modules/python3/pyportutils.h>
#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>
#include <modules/python3/pybindutils.h>

#include <map>
#include <string_view>
#include <algorithm>
#include <vector>

#include <fmt/base.h>

namespace inviwo::util {

template <typename T>
void exportDataSequenceFor(pybind11::module& m, std::string_view name) {
    namespace py = pybind11;

    py::classh<DataSequence<T>>(m, fmt::format("{}Sequence", name).c_str(), py::module_local(false))
        .def(py::init<>())
        .def(py::init<std::span<std::shared_ptr<const T>>>(), py::arg("volumes"))
        .def(py::init<const DataSequence<T>&>(), "Copy constructor")

        .def("__getitem__",
             [](const DataSequence<T>& v, std::ptrdiff_t i) -> std::shared_ptr<const T> {
                 if (i < 0) {
                     i += v.size();
                 }
                 if (i < 0 || static_cast<size_t>(i) >= v.size()) {
                     throw py::index_error();
                 }
                 return v[static_cast<size_t>(i)];
             })

        .def(
            "__iter__",
            [](const DataSequence<T>& v) {
                using ItType = DataSequence<T>::const_iterator;
                return py::make_iterator<py::return_value_policy::copy, ItType, ItType,
                                         std::shared_ptr<const T>>(v.begin(), v.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "__bool__", [](const DataSequence<T>& v) -> bool { return !v.empty(); },
            "Check whether the list is nonempty")
        .def("__len__", &DataSequence<T>::size)
        .def(
            "erase",
            [](DataSequence<T>& v, size_t i) {
                if (i >= v.size()) {
                    throw py::index_error();
                }
                v.erase(v.begin() + i);
            },
            "erases element at index ``i``")

        .def("empty", &DataSequence<T>::empty, "checks whether the container is empty")
        .def("size", &DataSequence<T>::size, "returns the number of elements")
        .def("push_back",
             static_cast<void (DataSequence<T>::*)(std::shared_ptr<const T>)>(
                 &DataSequence<T>::push_back),
             "adds an element to the end")
        .def("pop_back", &DataSequence<T>::pop_back, "removes the last element")

        .def("reserve", &DataSequence<T>::reserve, "reserves storage")
        .def("shrink_to_fit", &DataSequence<T>::shrink_to_fit,
             "reduces memory usage by freeing unused memory")

        .def("clear", &DataSequence<T>::clear, "clears the contents")
        .def(
            "front",
            [](DataSequence<T>& v) {
                if (!v.empty()) {
                    return v.front();
                } else {
                    throw py::index_error();
                }
            },
            "access the first element")

        .def(
            "back",
            [](DataSequence<T>& v) {
                if (!v.empty()) {
                    return v.back();
                } else {
                    throw py::index_error();
                }
            },
            "access the last element ")
        .def(
            "append",
            [](DataSequence<T>& v, std::shared_ptr<const T> value) { v.push_back(value); },
            py::arg("x"), "Add an item to the end of the list")
        .def(
            "extend",
            [](DataSequence<T>& v, const DataSequence<T>& src) {
                v.insert(v.end(), src.begin(), src.end());
            },
            py::arg("L"), "Extend the list by appending all the items in the given list")
        .def(
            "insert",
            [](DataSequence<T>& v, std::ptrdiff_t i, std::shared_ptr<const T> x) {
                // Can't use wrap_i; i == v.size() is OK
                if (i < 0) {
                    i += v.size();
                }
                if (i < 0 || static_cast<size_t>(i) > v.size()) {
                    throw py::index_error();
                }
                v.insert(v.begin() + i, x);
            },
            py::arg("i"), py::arg("x"), "Insert an item at a given position.");
}

}  // namespace inviwo::util
