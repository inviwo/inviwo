/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2025 Inviwo Foundation
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

#include <inviwo/core/util/glm.h>
#include <inviwopy/pyglmtypes.h>
#include <inviwo/core/util/ostreamjoiner.h>
#include <inviwo/core/util/assertion.h>
#include <modules/python3/pyportutils.h>
#include <modules/python3/opaquetypes.h>
#include <modules/python3/polymorphictypehooks.h>

#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/operators.h>
#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/numpy.h>
#include <warn/pop>

#include <map>
#include <string>
#include <algorithm>
#include <vector>

#include <warn/push>
#include <warn/ignore/self-assign-overloaded>

namespace inviwo {

namespace {

template <typename T, size_t N>
using ith_T = T;

template <typename V, typename T, std::size_t... I>
void addInitImpl(pybind11::classh<V>& pyv, std::index_sequence<I...>) {
    pyv.def(pybind11::init<ith_T<T, I>...>());
}

template <typename T, typename V, unsigned C, typename Indices = std::make_index_sequence<C>>
void addInit(pybind11::classh<V>& pyv) {
    addInitImpl<V, T>(pyv, Indices{});
}

}  // namespace

template <typename T, int Cols, int Rows>
void matxx(pybind11::module& m, const std::string& prefix, const std::string& name,
           const std::string& postfix) {

    using Mat = typename util::glmtype<T, Cols, Rows>::type;

    static_assert(std::is_standard_layout<Mat>::value, "has to be standard_layout");
    static_assert(std::is_trivially_copyable<Mat>::value, "has to be trivially_copyable");
    static_assert(pybind11::detail::is_pod_struct<Mat>::value, "has to be pod");

    using ColumnVector = typename Mat::col_type;
    using RowVector = typename Mat::row_type;

    using Mat2 = typename util::glmtype<T, 2, Cols>::type;
    using Mat3 = typename util::glmtype<T, 3, Cols>::type;
    using Mat4 = typename util::glmtype<T, 4, Cols>::type;

    const auto classname = [&]() {
        std::stringstream ss;
        ss << prefix << name << Cols;
        if (Cols != Rows) {
            ss << "x" << Rows;
        }
        ss << postfix;
        return ss.str();
    }();

    pybind11::classh<Mat> pym(m, classname.c_str(), pybind11::buffer_protocol{});
    addInit<T, Mat, Cols * Rows>(pym);
    addInit<ColumnVector, Mat, Cols>(pym);
    pym.def(pybind11::init<T>())
        .def(pybind11::init<Mat>())
        .def(pybind11::init<>())
        .def(pybind11::init([](pybind11::array_t<T> arr) {
            if (arr.ndim() != 2) throw std::invalid_argument{"Invalid dimensions"};
            if (arr.shape(0) != Cols) throw std::invalid_argument{"Invalid dimensions"};
            if (arr.shape(1) != Rows) throw std::invalid_argument{"Invalid dimensions"};

            Mat res;
            std::copy(arr.data(0), arr.data(0) + Cols * Rows, glm::value_ptr(res));
            return res;
        }))
        .def(pybind11::init([](pybind11::list list) {
            if (list.size() != Cols) {
                throw std::invalid_argument{
                    fmt::format("Invalid number of cols: got {}, expected {}", list.size(), Cols)};
            }
            Mat res;
            for (int i = 0; i < Cols; ++i) {
                auto col = list[i].cast<pybind11::list>();
                if (col.size() != Rows) {
                    throw std::invalid_argument{fmt::format(
                        "Invalid number of rows: got {}, expected {}", col.size(), Rows)};
                }
                for (int j = 0; j < Rows; ++j) {
                    res[i][j] = col[j].cast<T>();
                }
            }
            return res;
        }))
        .def(pybind11::self + pybind11::self)
        .def(pybind11::self - pybind11::self)
        .def(pybind11::self += pybind11::self)
        .def(pybind11::self -= pybind11::self)
        .def(pybind11::self == pybind11::self)
        .def(pybind11::self != pybind11::self)

        .def(pybind11::self + T())
        .def(pybind11::self - T())
        .def(pybind11::self * T())
        .def(pybind11::self / T())
        .def(pybind11::self += T())
        .def(pybind11::self -= T())
        .def(pybind11::self *= T())
        .def(pybind11::self /= T())

        .def(
            "__getitem__", [](Mat& v, int idx) { return &v[idx]; },
            pybind11::return_value_policy::reference_internal)
        .def("__getitem__", [](Mat& m, int idx, int idy) { return m[idx][idy]; })
        .def("__setitem__", [](Mat& m, int idx, ColumnVector& t) { return m[idx] = t; })
        .def("__setitem__", [](Mat& m, int idx, int idy, T& t) { return m[idx][idy] = t; })

        .def(pybind11::self * RowVector())
        .def(ColumnVector() * pybind11::self)
        .def(pybind11::self * Mat2())
        .def(pybind11::self * Mat3())
        .def(pybind11::self * Mat4())

        .def_property_readonly(
            "array",
            [](Mat& self) {
                return pybind11::array_t<T>(std::vector<size_t>{Rows, Cols}, glm::value_ptr(self));
            })

        .def("__repr__",
             [](Mat& m) {
                 std::ostringstream oss;
                 // oss << m; This fails for some reason on GCC 5.4

                 oss << "[";
                 for (int col = 0; col < Cols; col++) {
                     oss << "[";
                     for (int row = 0; row < Rows; row++) {
                         if (row != 0) oss << " ";
                         oss << m[col][row];
                     }
                     oss << "]";
                 }
                 oss << "]";

                 return oss.str();
             })
        .def_buffer([](Mat& mat) -> pybind11::buffer_info {
            return pybind11::buffer_info(
                glm::value_ptr(mat),                      /* Pointer to buffer */
                sizeof(T),                                /* Size of one scalar */
                pybind11::format_descriptor<T>::format(), /* Python struct-style format descriptor
                                                           */
                2,                                        /* Number of dimensions */
                {Rows, Cols},                             /* Buffer dimensions */
                {sizeof(T), sizeof(T) * Rows}             /* Strides (in bytes) for each index */
            );
        });

    pybind11::bind_vector<std::vector<Mat>>(m, classname + "Vector", "Vectors of glm matrices");

    pybind11::implicitly_convertible<pybind11::list, Mat>();
    pybind11::implicitly_convertible<pybind11::array_t<T>, Mat>();
}

template <typename T, int Cols>
void matx(pybind11::module& m, const std::string& prefix, const std::string& name,
          const std::string& postfix) {
    matxx<T, Cols, 2>(m, prefix, name, postfix);
    matxx<T, Cols, 3>(m, prefix, name, postfix);
    matxx<T, Cols, 4>(m, prefix, name, postfix);
}

template <typename T>
void mat(pybind11::module& m, const std::string& prefix, const std::string& name = "mat",
         const std::string& postfix = "") {
    matx<T, 2>(m, prefix, name, postfix);
    matx<T, 3>(m, prefix, name, postfix);
    matx<T, 4>(m, prefix, name, postfix);
}

namespace {

template <typename T>
constexpr bool alwaysFalse() {
    return false;
}

}  // namespace

struct ExposePortsFunctor {
    template <typename T>
    void operator()(pybind11::module& m) {
        using V = util::value_type_t<T>;

        constexpr auto rank = util::rank_v<T>;

        if constexpr (rank == 0) {
            constexpr auto name = []() {
                if constexpr (std::is_same_v<T, float>) {
                    return "float";
                } else if constexpr (std::is_same_v<T, double>) {
                    return "double";
                } else if constexpr (std::is_same_v<T, int>) {
                    return "int";
                } else if constexpr (std::is_same_v<T, unsigned int>) {
                    return "uint";
                } else {
                    static_assert(alwaysFalse<T>(), "Missing name for T");
                }
            }();
            const auto vectorName = fmt::format("{}Vector", name);
            pybind11::bind_vector<std::vector<T>>(m, vectorName);
            exposeStandardDataPorts<std::vector<T>>(m, vectorName);

        } else if constexpr (rank == 1 && util::extent_v<T> <= 4) {
            constexpr auto N = util::extent_v<T>;
            const auto prefix = glm::detail::prefix<V>::value();
            const auto vectorName = fmt::format("{}vec{}Vector", prefix, N);
            pybind11::bind_vector<std::vector<T>>(m, vectorName, pybind11::buffer_protocol{});
            exposeStandardDataPorts<std::vector<T>>(m, vectorName);

        } else if constexpr (rank == 2 && util::extent_v<T, 0> <= 4 &&
                             util::extent_v<T, 0> == util::extent_v<T, 1>) {
            constexpr auto N = util::extent_v<T, 0>;
            const auto prefix = glm::detail::prefix<V>::value();
            const auto vectorName = fmt::format("{}mat{}Vector", prefix, N);
            exposeStandardDataPorts<std::vector<T>>(m, vectorName);
        } else {
            static_assert(alwaysFalse<T>(), "T not supported");
        }
    }
};

}  // namespace inviwo

#include <warn/pop>
