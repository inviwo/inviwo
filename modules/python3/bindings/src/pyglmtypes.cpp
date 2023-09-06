/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2023 Inviwo Foundation
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
#include <inviwopy/pyglmmattypes.h>

#include <inviwo/core/util/ostreamjoiner.h>
#include <inviwo/core/util/logcentral.h>
#include <modules/python3/pyportutils.h>

#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/operators.h>
#include <pybind11/numpy.h>
#include <pybind11/stl_bind.h>
#include <warn/pop>

#include <map>
#include <string>
#include <algorithm>

#include <fmt/format.h>

#include <warn/push>
#include <warn/ignore/self-assign-overloaded>

namespace inviwo {

namespace py = pybind11;

namespace {

template <typename T, size_t N>
using ith_T = T;

template <typename V, typename T, std::size_t... I>
void addInitImpl(py::class_<V>& pyv, std::index_sequence<I...>) {
    pyv.def(py::init<ith_T<T, I>...>());
}

template <typename T, typename V, unsigned C, typename Indices = std::make_index_sequence<C>>
void addInit(py::class_<V>& pyv) {
    addInitImpl<V, T>(pyv, Indices{});
}
}  // namespace

template <typename T, int Dim>
void vecx(py::module& m, const std::string& prefix, const std::string& name,
          const std::string& postfix) {
    using Vec = glm::vec<Dim, T>;

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
        .def(py::init<Vec>())
        .def(py::init<>())
        .def(py::init([](py::array_t<T> arr) {
            if (arr.ndim() != 1)
                throw std::invalid_argument{
                    fmt::format("Invalid ndim: got {}, expected {}", arr.ndim(), 1)};
            if (arr.shape(0) != Dim)
                throw std::invalid_argument{
                    fmt::format("Invalid shape: got {}, expected {}", arr.shape(0), Dim)};

            Vec res;
            std::copy(arr.data(0), arr.data(0) + Dim, glm::value_ptr(res));
            return res;
        }))
        .def(py::init([](py::list list) {
            if (list.size() != Dim)
                throw std::invalid_argument{
                    fmt::format("Invalid size: got {}, expected {}", list.size(), Dim)};

            Vec res;
            for (int i = 0; i < Dim; ++i) {
                res[i] = list[i].cast<T>();
            }
            return res;
        }))

        .def(py::self == py::self)
        .def(py::self != py::self)

        .def(
            "__getitem__", [](Vec& v, int idx) { return &v[idx]; },
            py::return_value_policy::reference_internal)
        .def("__setitem__", [](Vec& v, int idx, T& t) { return v[idx] = t; })

        .def_property_readonly("array",
                               [](Vec& self) { return py::array_t<T>(Dim, glm::value_ptr(self)); })
        .def("__repr__",
             [](Vec& v) {
                 std::ostringstream oss;
                 // oss << v; This fails for some reason on GCC 5.4

                 oss << "[";
                 std::copy(glm::value_ptr(v), glm::value_ptr(v) + Dim,
                           util::make_ostream_joiner(oss, " "));
                 oss << "]";
                 return std::move(oss).str();
             })

        .def_buffer([](Vec& vec) -> py::buffer_info {
            return py::buffer_info(
                glm::value_ptr(vec),                /* Pointer to buffer */
                sizeof(T),                          /* Size of one scalar */
                py::format_descriptor<T>::format(), /* Python struct-style format descriptor */
                1,                                  /* Number of dimensions */
                {Dim},                              /* Buffer dimensions */
                {sizeof(T)}                         /* Strides (in bytes) for each index */
            );
        });

    if constexpr (Dim == 2) {
        pyv.def(py::init<glm::vec<3, T>>());
        pyv.def(py::init<glm::vec<4, T>>());
    }
    if constexpr (Dim == 3) {
        pyv.def(py::init<glm::vec<2, T>, T>());
        pyv.def(py::init<T, glm::vec<2, T>>());

        pyv.def(py::init<glm::vec<4, T>>());
    }
    if constexpr (Dim == 4) {
        pyv.def(py::init<glm::vec<2, T>, T, T>());
        pyv.def(py::init<T, glm::vec<2, T>, T>());
        pyv.def(py::init<T, T, glm::vec<2, T>>());
        pyv.def(py::init<glm::vec<2, T>, glm::vec<2, T>>());

        pyv.def(py::init<glm::vec<3, T>, T>());
        pyv.def(py::init<T, glm::vec<3, T>>());
    }

    if constexpr (std::is_same_v<T, bool>) {
        m.def("all", [](const Vec& v1) { return glm::all(v1); });
        m.def("any", [](const Vec& v1) { return glm::any(v1); });
    } else {
        pyv.def(py::self * py::self)
            .def(py::self / py::self)
            .def(py::self *= py::self)
            .def(py::self /= py::self)
            .def(py::self + py::self)
            .def(py::self - py::self)
            .def(py::self += py::self)
            .def(py::self -= py::self)
            .def(py::self + T())
            .def(py::self - T())
            .def(py::self * T())
            .def(py::self / T())
            .def(py::self += T())
            .def(py::self -= T())
            .def(py::self *= T())
            .def(py::self /= T());

        m.def("clamp",
              [](const Vec& v1, const Vec& v2, const Vec& v3) { return glm::clamp(v1, v2, v3); });
        m.def("clamp", [](const Vec& v1, T& v2, T& v3) { return glm::clamp(v1, v2, v3); });
        m.def("compAdd", [](const Vec& v1) { return glm::compAdd(v1); });
        m.def("compMax", [](const Vec& v1) { return glm::compMax(v1); });
        m.def("compMin", [](const Vec& v1) { return glm::compMin(v1); });
        m.def("compMul", [](const Vec& v1) { return glm::compMul(v1); });
        m.def("abs", [](const Vec& v) { return glm::abs(v); });

        m.def("lessThan", [](const Vec& v1, const Vec& v2) { return glm::lessThan(v1, v2); });
        m.def("lessThanEqual",
              [](const Vec& v1, const Vec& v2) { return glm::lessThanEqual(v1, v2); });
        m.def("greaterThan", [](const Vec& v1, const Vec& v2) { return glm::greaterThan(v1, v2); });
        m.def("greaterThanEqual",
              [](const Vec& v1, const Vec& v2) { return glm::greaterThanEqual(v1, v2); });
    }

    if constexpr (std::is_floating_point_v<T>) {
        m.def("acos", [](const Vec& v1) { return glm::acos(v1); });
        m.def("asin", [](const Vec& v1) { return glm::asin(v1); });
        m.def("atan", [](const Vec& v1) { return glm::atan(v1); });
        m.def("ceil", [](const Vec& v1) { return glm::ceil(v1); });
        m.def("cos", [](const Vec& v1) { return glm::cos(v1); });
        m.def("distance", [](const Vec& v1, const Vec& v2) { return glm::distance(v1, v2); });
        m.def("distance2", [](const Vec& v1, const Vec& v2) { return glm::distance2(v1, v2); });
        m.def("dot", [](const Vec& v1, const Vec& v2) { return glm::dot(v1, v2); });
        m.def("exp", [](const Vec& v1) { return glm::exp(v1); });
        m.def("floor", [](const Vec& v1) { return glm::floor(v1); });
        m.def("length", [](const Vec& v) { return glm::length(v); });
        m.def("length2", [](const Vec& v) { return glm::length2(v); });
        m.def("normalize", [](const Vec& v) { return glm::normalize(v); });
        m.def("sin", [](const Vec& v1) { return glm::sin(v1); });
        m.def("tan", [](const Vec& v1) { return glm::tan(v1); });

        if constexpr (Dim == 3) {
            m.def("cross", [](const Vec& v1, const Vec& v2) { return glm::cross(v1, v2); });

            m.def("rotate", [](const glm::mat<4, 4, T>& m, T angle, Vec& n) {
                return glm::rotate(m, angle, n);
            });

            m.def("rotate", [](const Vec& v, T angle, Vec& n) { return glm::rotate(v, angle, n); });
        }
    }

    try {
        if constexpr (Dim == 1) {
            PYBIND11_NUMPY_DTYPE(Vec, x);
        } else if constexpr (Dim == 2) {
            PYBIND11_NUMPY_DTYPE(Vec, x, y);
        } else if constexpr (Dim == 3) {
            PYBIND11_NUMPY_DTYPE(Vec, x, y, z);
        } else if constexpr (Dim == 4) {
            PYBIND11_NUMPY_DTYPE(Vec, x, y, z, w);
        }
    } catch (const std::exception& e) {
        util::log(IVW_CONTEXT_CUSTOM("pyglmtypes"), e.what());
    }

    switch (Dim) {
        case 4:
            pyv.def_property(
                "w", [](Vec& b) { return b[3]; }, [](Vec& b, T t) { b[3] = t; });
            pyv.def_property(
                "a", [](Vec& b) { return b[3]; }, [](Vec& b, T t) { b[3] = t; });
            pyv.def_property(
                "q", [](Vec& b) { return b[3]; }, [](Vec& b, T t) { b[3] = t; });
            [[fallthrough]];
        case 3:
            pyv.def_property(
                "z", [](Vec& b) { return b[2]; }, [](Vec& b, T t) { b[2] = t; });
            pyv.def_property(
                "b", [](Vec& b) { return b[2]; }, [](Vec& b, T t) { b[2] = t; });
            pyv.def_property(
                "p", [](Vec& b) { return b[2]; }, [](Vec& b, T t) { b[2] = t; });
            [[fallthrough]];
        case 2:
            pyv.def_property(
                "y", [](Vec& b) { return b[1]; }, [](Vec& b, T t) { b[1] = t; });
            pyv.def_property(
                "g", [](Vec& b) { return b[1]; }, [](Vec& b, T t) { b[1] = t; });
            pyv.def_property(
                "t", [](Vec& b) { return b[1]; }, [](Vec& b, T t) { b[1] = t; });
            pyv.def_property(
                "x", [](Vec& b) { return b[0]; }, [](Vec& b, T t) { b[0] = t; });
            pyv.def_property(
                "r", [](Vec& b) { return b[0]; }, [](Vec& b, T t) { b[0] = t; });
            pyv.def_property(
                "s", [](Vec& b) { return b[0]; }, [](Vec& b, T t) { b[0] = t; });
            [[fallthrough]];
        default:
            break;
    }

    py::implicitly_convertible<py::list, Vec>();
    py::implicitly_convertible<py::array_t<T>, Vec>();
}

template <typename T>
void vec(py::module& m, const std::string& prefix, const std::string& name = "vec",
         const std::string& postfix = "") {
    vecx<T, 2>(m, prefix, name, postfix);
    vecx<T, 3>(m, prefix, name, postfix);
    vecx<T, 4>(m, prefix, name, postfix);
}

void exposeGLMTypes(py::module& m) {
    vec<bool>(m, "b");
    vec<float>(m, "");
    vec<double>(m, "d");
    vec<int>(m, "i");
    vec<unsigned int>(m, "u");
    vec<size_t>(m, "", "size", "_t");
}
}  // namespace inviwo

#include <warn/pop>
