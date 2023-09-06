/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2023 Inviwo Foundation
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
#include <warn/pop>

#include <inviwo/core/util/ostreamjoiner.h>

#include <memory>
#include <sstream>
#include <cstdint>
#include <algorithm>
#include <iterator>
#include <ranges>

namespace inviwo {

template <typename It, bool Deref = true>
class VectorIdentifierWrapper {

    using value_type = typename std::iterator_traits<It>::value_type;

    static decltype(auto) id(const value_type& elem) {
        if constexpr (Deref) {
            return elem->getIdentifier();
        } else {
            return elem.getIdentifier();
        }
    }

    static decltype(auto) deref(It it) {
        if constexpr (Deref) {
            return *(*it);
        } else {
            return *it;
        }
    }

public:
    VectorIdentifierWrapper(It begin, It end) : begin_{begin}, end_{end} {}

    decltype(auto) getFromIdentifier(std::string_view identifier) const {
        auto it =
            std::find_if(begin_, end_, [&](const auto& elem) { return id(elem) == identifier; });
        if (it != end_) {
            return deref(it);
        } else {
            throw pybind11::key_error();
        }
    }

    decltype(auto) getFromIndex(ptrdiff_t pos) const {
        if (pos >= 0) {
            if (pos < size()) {
                return deref(std::next(begin_, pos));
            } else {
                throw pybind11::index_error();
            }
        } else {
            const auto ind = size() + pos;
            if (ind >= 0) {
                return deref(std::next(begin_, ind));
            } else {
                throw pybind11::index_error();
            }
        }
    }

    ptrdiff_t size() const { return static_cast<ptrdiff_t>(std::distance(begin_, end_)); }

    bool contains(std::string_view identifier) const {
        return std::find_if(begin_, end_,
                            [&](const auto& elem) { return id(elem) == identifier; }) != end_;
    }

    std::vector<std::string> identifiers() const {
        std::vector<std::string> res;
        std::transform(begin_, end_, std::back_inserter(res), id);
        return res;
    }

    std::string repr() const {
        std::stringstream ss;
        ss << "[";
        auto j = util::make_ostream_joiner(ss, ", ");
        std::transform(begin_, end_, j, id);
        ss << "]";
        return ss.str();
    }

private:
    It begin_;
    It end_;
};

template <typename V, bool Deref = true>
void exposeVectorIdentifierWrapper(pybind11::module& m, const std::string& name) {
    namespace py = pybind11;
    using VecWrapper = VectorIdentifierWrapper<V, Deref>;

    py::class_<VecWrapper>(m, name.c_str())
        .def("__getattr__", &VecWrapper::getFromIdentifier, py::return_value_policy::reference)
        .def("__getitem__", &VecWrapper::getFromIdentifier, py::return_value_policy::reference)
        .def("__getitem__", &VecWrapper::getFromIndex, py::return_value_policy::reference)
        .def("__len__", &VecWrapper::size)
        .def("__contains__", &VecWrapper::contains)
        .def("__repr__", &VecWrapper::repr)
        .def("__dir__", &VecWrapper::identifiers);
}

}  // namespace inviwo
