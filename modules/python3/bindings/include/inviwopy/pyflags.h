/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2021 Inviwo Foundation
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
#include <pybind11/numpy.h>
#include <pybind11/operators.h>
#include <warn/pop>

#include <inviwo/core/interaction/pickingstate.h>
#include <inviwo/core/util/safecstr.h>
#include <inviwo/core/util/typetraits.h>

#include <flags/flags.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

namespace inviwo {

#include <warn/push>
#include <warn/ignore/self-assign-overloaded>

template <typename E>
void exposeFlags(pybind11::module& m, std::string_view name) {
    namespace py = pybind11;

    using Iter = typename flags::flags<E>::iterator;

    py::class_<Iter>(m, fmt::format("{}Iterator", name).c_str())
        .def(py::init<Iter>())
        .def("__iter__", [](Iter& self) { return Iter{self}; })
        .def("__next__", [](Iter& self) {
            if (self == Iter{}) {
                throw py::stop_iteration{};
            } else {
                return *(self++);
            }
        });

    py::class_<flags::flags<E>> flags(m, SafeCStr{name}.c_str());
    flags.def(py::init([]() { return flags::flags<E>{flags::empty}; }))
        .def(py::init<E>())
        .def(py::init<flags::flags<E>>())
        .def(py::init([](py::args args) {
            auto f = flags::flags<E>{flags::empty};
            for (auto i : args.cast<std::vector<E>>()) {
                f.insert(i);
            }
            return f;
        }))

        .def(py::self |= py::self)
        .def(py::self &= py::self)
        .def(py::self ^= py::self)

        .def(py::self |= E())
        .def(py::self &= E())
        .def(py::self ^= E())

        .def(py::self | py::self)
        .def(py::self & py::self)
        .def(py::self ^ py::self)

        .def("empty", &flags::flags<E>::empty)
        .def("size", &flags::flags<E>::size)
        .def("max_size", &flags::flags<E>::max_size)
        .def("find", &flags::flags<E>::find)
        .def("count", &flags::flags<E>::count)
        .def("contains", &flags::flags<E>::contains)
        .def("insert",
             static_cast<std::pair<Iter, bool> (flags::flags<E>::*)(E e)>(&flags::flags<E>::insert))
        .def("erase", static_cast<size_t (flags::flags<E>::*)(E e)>(&flags::flags<E>::erase))
        .def("clear", &flags::flags<E>::clear)
        .def("empty", &flags::flags<E>::empty)
        .def("__iter__", &flags::flags<E>::begin);

    if constexpr (util::is_stream_insertable<flags::flags<E>>::value) {
        flags.def("__str__", [name](const flags::flags<E>& e) {
            if (e.empty()) {
                return fmt::format("{}.{}", name, E{0});
            } else {
                return fmt::format("{}.{}", name, e);
            }
        });
        flags.def("__repr__", [name](const flags::flags<E>& e) {
            if (e.empty()) {
                return fmt::format("<{}.{}: {}>", name, E{0}, e.underlying_value());
            } else {
                return fmt::format("<{}.{}: {}>", name, e, e.underlying_value());
            }
        });
    } else {
        flags.def("__str__", [name](const flags::flags<E>& e) {
            return fmt::format("{}.{}", name, e.underlying_value());
        });
        flags.def("__repr__", [name](const flags::flags<E>& e) {
            return fmt::format("<{}: {}>", name, e.underlying_value());
        });
    }
}

#include <warn/pop>

}  // namespace inviwo
