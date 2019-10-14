/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <inviwopy/pyglmtypes.h>

#include <inviwo/core/util/ostreamjoiner.h>

#include <inviwopy/pyglmmattypes.h>

#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/operators.h>
#include <pybind11/numpy.h>
#include <pybind11/stl_bind.h>
#include <warn/pop>

#include <map>
#include <string>
#include <algorithm>

#include <warn/push>
#include <warn/ignore/self-assign-overloaded>

namespace inviwo {

namespace py = pybind11;

namespace {

template <typename T, size_t N>
using ith_T = T;

template <typename V, typename T, std::size_t... I>
void addInitImpl(py::class_<V> &pyv, std::index_sequence<I...>) {
    pyv.def(py::init<ith_T<T, I>...>());
}

template <typename T, typename V, unsigned C, typename Indices = std::make_index_sequence<C>>
void addInit(py::class_<V> &pyv) {
    addInitImpl<V, T>(pyv, Indices{});
}

template <typename T, size_t N>
struct dtype {};

template <typename Vec>
struct dtype<Vec, 1> {
    static void init() { PYBIND11_NUMPY_DTYPE(Vec, x); }
};
template <typename Vec>
struct dtype<Vec, 2> {
    static void init() { PYBIND11_NUMPY_DTYPE(Vec, x, y); }
};
template <typename Vec>
struct dtype<Vec, 3> {
    static void init() { PYBIND11_NUMPY_DTYPE(Vec, x, y, z); }
};
template <typename Vec>
struct dtype<Vec, 4> {
    static void init() { PYBIND11_NUMPY_DTYPE(Vec, x, y, z, w); }
};

}  // namespace

template <typename T, typename V>
void floatOnlyVecs(py::module &, std::false_type) {}

template <typename T, typename V>
void floatOnlyVecs(py::module &m, std::true_type) {
    m.def("dot", [](V &v, V &v2) { return glm::dot(v, v2); });
    m.def("distance", [](V &v, V &v2) { return glm::distance(v, v2); });
    m.def("distance2", [](V &v, V &v2) { return glm::distance2(v, v2); });
    m.def("length", [](V &v) { return glm::length(v); });
    m.def("length2", [](V &v) { return glm::length2(v); });
    m.def("normalize", [](V &v) { return glm::normalize(v); });
}

template <typename T, typename V>
void floatOnlyVecs(py::module &m) {
    floatOnlyVecs<T, V>(m, std::is_floating_point<T>());
}

template <typename T, int Dim>
void vecx(py::module &m, const std::string &prefix, const std::string &name,
          const std::string &postfix) {
    using Vec = Vector<Dim, T>;

    static_assert(std::is_standard_layout<Vec>::value, "has to be standard_layout");
    static_assert(std::is_trivially_copyable<Vec>::value, "has to be trivially_copyable");
    static_assert(py::detail::is_pod_struct<Vec>::value, "has to be pod");

    const auto classname = [&]() {
        std::stringstream ss;
        ss << prefix << name << Dim << postfix;
        return ss.str();
    }();

    py::class_<Vec> pyv(m, classname.c_str(), py::buffer_protocol{});
    addInit<T, Vec, Dim>(pyv);
    pyv.def(py::init<T>())
        .def(py::init<>())
        .def(py::init([](py::array_t<T> arr) {
            if (arr.ndim() != 1) throw std::invalid_argument{"Invalid dimensions"};
            if (arr.shape(0) != Dim) throw std::invalid_argument{"Invalid dimensions"};

            Vec res;
            std::copy(arr.data(0), arr.data(0) + Dim, glm::value_ptr(res));
            return res;
        }))
        .def(py::self * py::self)
        .def(py::self / py::self)
        .def(py::self *= py::self)
        .def(py::self /= py::self)
        .def(py::self + py::self)
        .def(py::self - py::self)
        .def(py::self += py::self)
        .def(py::self -= py::self)
        .def(py::self == py::self)
        .def(py::self != py::self)

        .def(py::self + T())
        .def(py::self - T())
        .def(py::self * T())
        .def(py::self / T())
        .def(py::self += T())
        .def(py::self -= T())
        .def(py::self *= T())
        .def(py::self /= T())

        .def("__getitem__", [](Vec &v, int idx) { return &v[idx]; },
             py::return_value_policy::reference_internal)
        .def("__setitem__", [](Vec &v, int idx, T &t) { return v[idx] = t; })

        .def_property_readonly("array",
                               [](Vec &self) { return py::array_t<T>(Dim, glm::value_ptr(self)); })
        .def("__repr__",
             [](Vec &v) {
                 std::ostringstream oss;
                 // oss << v; This fails for some reason on GCC 5.4

                 oss << "[";
                 std::copy(glm::value_ptr(v), glm::value_ptr(v) + Dim,
                           util::make_ostream_joiner(oss, " "));
                 oss << "]";
                 return std::move(oss).str();
             })

        .def_buffer([](Vec &vec) -> py::buffer_info {
            return py::buffer_info(
                glm::value_ptr(vec),                /* Pointer to buffer */
                sizeof(T),                          /* Size of one scalar */
                py::format_descriptor<T>::format(), /* Python struct-style format descriptor */
                1,                                  /* Number of dimensions */
                {Dim},                              /* Buffer dimensions */
                {sizeof(T)}                         /* Strides (in bytes) for each index */
            );
        });

    floatOnlyVecs<T, Vec>(m);

    dtype<Vec, Dim>::init();
    py::bind_vector<std::vector<Vec>>(m, classname + "Vector", py::buffer_protocol{});

    switch (Dim) {
        case 4:
            pyv.def_property("w", [](Vec &b) { return b[3]; }, [](Vec &b, T t) { b[3] = t; });
            pyv.def_property("a", [](Vec &b) { return b[3]; }, [](Vec &b, T t) { b[3] = t; });
            pyv.def_property("q", [](Vec &b) { return b[3]; }, [](Vec &b, T t) { b[3] = t; });
            [[fallthrough]];
        case 3:
            pyv.def_property("z", [](Vec &b) { return b[2]; }, [](Vec &b, T t) { b[2] = t; });
            pyv.def_property("b", [](Vec &b) { return b[2]; }, [](Vec &b, T t) { b[2] = t; });
            pyv.def_property("p", [](Vec &b) { return b[2]; }, [](Vec &b, T t) { b[2] = t; });
            [[fallthrough]];
        case 2:
            pyv.def_property("y", [](Vec &b) { return b[1]; }, [](Vec &b, T t) { b[1] = t; });
            pyv.def_property("g", [](Vec &b) { return b[1]; }, [](Vec &b, T t) { b[1] = t; });
            pyv.def_property("t", [](Vec &b) { return b[1]; }, [](Vec &b, T t) { b[1] = t; });
            pyv.def_property("x", [](Vec &b) { return b[0]; }, [](Vec &b, T t) { b[0] = t; });
            pyv.def_property("r", [](Vec &b) { return b[0]; }, [](Vec &b, T t) { b[0] = t; });
            pyv.def_property("s", [](Vec &b) { return b[0]; }, [](Vec &b, T t) { b[0] = t; });
            [[fallthrough]];
        default:
            break;
    }
}

template <typename T>
void vec(py::module &m, const std::string &prefix, const std::string &name = "vec",
         const std::string &postfix = "") {
    vecx<T, 2>(m, prefix, name, postfix);
    vecx<T, 3>(m, prefix, name, postfix);
    vecx<T, 4>(m, prefix, name, postfix);
}

void exposeGLMTypes(py::module &m) {
    auto glmModule = m.def_submodule("glm", "Exposing glm vec and mat types");

    vec<float>(glmModule, "");
    vec<double>(glmModule, "d");
    vec<int>(glmModule, "i");
    vec<unsigned int>(glmModule, "u");
    vec<size_t>(glmModule, "", "size", "_t");

    exposeGLMMatTypes(glmModule);
}
}  // namespace inviwo

#include <warn/pop>
