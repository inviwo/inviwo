/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#ifndef IVW_VECTORIDENTIFIERWRAPPER_H
#define IVW_VECTORIDENTIFIERWRAPPER_H

#include <inviwo/core/common/inviwo.h>

#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/pybind11.h>
#include <warn/pop>
#include <memory>
#include <sstream>
#include <inviwo/core/util/ostreamjoiner.h>

namespace inviwo {

template <typename V>
class VectorIdentifierWrapper {
public:
    VectorIdentifierWrapper(const V& vector) : vector_{vector} {}

    auto& getFromIdentifier(const std::string& identifier) const {
        using std::begin;
        using std::end;
        auto it = std::find_if(begin(vector_), end(vector_), [&](const auto& elem) {
            return elem->getIdentifier() == identifier;
        });
        if (it != end(vector_)) {
            return *(*it);
        } else {
            throw pybind11::key_error();
        }
    }

    auto& getFromIndex(size_t ind) const {
        if (ind < vector_.size()) {
            return *vector_[ind];
        } else {
            throw pybind11::index_error();
        }
    }

    size_t size() { return vector_.size(); }

    bool contains(const std::string& identifier) {
        return std::find_if(begin(vector_), end(vector_), [&](const auto& elem) {
                   return elem->getIdentifier() == identifier;
               }) != end(vector_);
    }

    std::string repr() const {
        std::stringstream ss;
        ss << "[";
        auto j = util::make_ostream_joiner(ss, ", ");
        std::transform(vector_.begin(), vector_.end(), j,
                       [](auto& elem) { return elem->getIdentifier(); });
        ss << "]";
        return ss.str();
    }

private:
    const V& vector_;
};

template <typename V>
void exposeVectorIdentifierWrapper(pybind11::module& m, const std::string& name) {
    namespace py = pybind11;
    using VecWrapper = VectorIdentifierWrapper<V>;

    py::class_<VecWrapper>(m, name.c_str())
        .def("__getattr__", &VecWrapper::getFromIdentifier, py::return_value_policy::reference)
        .def("__getitem__", &VecWrapper::getFromIndex, py::return_value_policy::reference)
        .def("__len__", &VecWrapper::size)
        .def("__contains__", &VecWrapper::contains)
        .def("__repr__", &VecWrapper::repr);
}

}  // namespace inviwo

#endif  // IVW_VECTORIDENTIFIERWRAPPER_H
