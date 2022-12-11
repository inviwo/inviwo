/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwopy/pybitset.h>

#include <inviwo/core/datastructures/bitset.h>  // for BitSet

#include <pybind11/numpy.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

#include <fmt/format.h>

namespace inviwo {

#include <warn/push>
#include <warn/ignore/self-assign-overloaded>

namespace {

struct BitSetIteratorWrapper {
    BitSet::BitSetIterator begin;
    BitSet::BitSetIterator end;
};

}  // namespace

void exposeBitset(pybind11::module& m) {
    namespace py = pybind11;

    py::class_<BitSetIteratorWrapper>(m, "BitSetIterator")
        .def("__next__", [](BitSetIteratorWrapper& w) {
            if (w.begin != w.end) {
                auto val = *w.begin;
                ++w.begin;
                return val;
            } else {
                throw pybind11::stop_iteration();
            }
        });

    py::class_<BitSet>(m, "BitSet")
        .def(py::init<>())
        .def(py::init<const std::vector<bool>&>())
        .def(py::init([](py::list list){
            BitSet s{};
            for(auto& i : list) {
                s.add(i.cast<uint32_t>());
            }
            return s;
        }))

        .def("cardinality", &BitSet::cardinality)
        .def("size", &BitSet::size)
        .def("empty", &BitSet::empty)
        .def("clear", &BitSet::clear)
        .def("isSubsetOf", &BitSet::isSubsetOf)
        .def("isStrictSubsetOf", &BitSet::isStrictSubsetOf)
        .def("set", &BitSet::set)
        .def("add", static_cast<void (BitSet::*)(const std::vector<bool>&)>(&BitSet::add))
        .def("addChecked", &BitSet::addChecked)
        .def("addRange", &BitSet::addRange)
        .def("addRangeClosed", &BitSet::addRangeClosed)
        .def("remove", &BitSet::remove)
        .def("removeChecked", &BitSet::removeChecked)
        .def("max", &BitSet::max)
        .def("min", &BitSet::min)
        .def("contains", &BitSet::contains)
        .def("containsRange", &BitSet::containsRange)
        .def("flip", &BitSet::flip)
        .def("flipRange", &BitSet::flipRange)
        .def("rank", &BitSet::rank)
        .def("intersect", &BitSet::intersect)
        .def("orCardinality", &BitSet::orCardinality)
        .def("andCardinality", &BitSet::andCardinality)
        .def("andNotCardinality", &BitSet::andNotCardinality)
        .def("xorCardinality", &BitSet::xorCardinality)
        .def("jaccardIndex", &BitSet::jaccardIndex)
        .def("fastUnion", &BitSet::fastUnion)
        .def("toVector", &BitSet::toVector)
        .def("getSizeInBytes", &BitSet::getSizeInBytes)
        .def("optimize", &BitSet::optimize)
        .def("removeRLECompression", &BitSet::removeRLECompression)
        .def("shrinkToFit", &BitSet::shrinkToFit)

        .def(py::self == py::self)
        .def(py::self != py::self)
        .def(py::self &= py::self)
        .def(py::self & py::self)
        .def(py::self -= py::self)
        .def(py::self - py::self)
        .def(py::self |= py::self)
        .def(py::self | py::self)
        .def(py::self ^= py::self)
        .def(py::self ^ py::self)

        .def("__iter__",
             [](BitSet& b) {
                 return BitSetIteratorWrapper{b.begin(), b.end()};
             }, py::keep_alive<0, 1>())
        .def("__len__", &BitSet::size)
        .def("__contains__", &BitSet::contains);
}

#include <warn/pop>

}  // namespace inviwo
