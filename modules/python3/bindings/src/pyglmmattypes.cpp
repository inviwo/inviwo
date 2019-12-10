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

#include <inviwopy/pyglmmattypes.h>
#include <inviwopy/pyglmtypes.h>

#include <inviwo/core/util/ostreamjoiner.h>

#include <pybind11/operators.h>
#include <pybind11/numpy.h>
#include <pybind11/stl_bind.h>

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

}  // namespace

template <typename T, int Cols, int Rows>
void matxx(py::module &m, const std::string &prefix, const std::string &name,
           const std::string &postfix) {

    using Mat = typename util::glmtype<T, Cols, Rows>::type;

    static_assert(std::is_standard_layout<Mat>::value, "has to be standard_layout");
    static_assert(std::is_trivially_copyable<Mat>::value, "has to be trivially_copyable");
    static_assert(py::detail::is_pod_struct<Mat>::value, "has to be pod");

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

    py::class_<Mat> pym(m, classname.c_str(), py::buffer_protocol{});
    addInit<T, Mat, Cols * Rows>(pym);
    addInit<ColumnVector, Mat, Cols>(pym);
    pym.def(py::init<T>())
        .def(py::init<>())
        .def(py::init([](py::array_t<T> arr) {
            if (arr.ndim() != 2) throw std::invalid_argument{"Invalid dimensions"};
            if (arr.shape(0) != Cols) throw std::invalid_argument{"Invalid dimensions"};
            if (arr.shape(1) != Rows) throw std::invalid_argument{"Invalid dimensions"};

            Mat res;
            std::copy(arr.data(0), arr.data(0) + Cols * Rows, glm::value_ptr(res));
            return res;
        }))
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

        .def("__getitem__", [](Mat &v, int idx) { return &v[idx]; },
             py::return_value_policy::reference_internal)
        .def("__getitem__", [](Mat &m, int idx, int idy) { return m[idx][idy]; })
        .def("__setitem__", [](Mat &m, int idx, ColumnVector &t) { return m[idx] = t; })
        .def("__setitem__", [](Mat &m, int idx, int idy, T &t) { return m[idx][idy] = t; })

        .def(py::self * RowVector())
        .def(ColumnVector() * py::self)
        .def(py::self * Mat2())
        .def(py::self * Mat3())
        .def(py::self * Mat4())

        .def_property_readonly(
            "array",
            [](Mat &self) {
                return py::array_t<T>(std::vector<size_t>{Rows, Cols}, glm::value_ptr(self));
            })

        .def("__repr__",
             [](Mat &m) {
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
        .def_buffer([](Mat &mat) -> py::buffer_info {
            return py::buffer_info(
                glm::value_ptr(mat),                /* Pointer to buffer */
                sizeof(T),                          /* Size of one scalar */
                py::format_descriptor<T>::format(), /* Python struct-style format descriptor */
                2,                                  /* Number of dimensions */
                {Rows, Cols},                       /* Buffer dimensions */
                {sizeof(T), sizeof(T) * Rows}       /* Strides (in bytes) for each index */
            );
        });

    py::bind_vector<std::vector<Mat>>(m, classname + "Vector", "Vectors of glm matrices");
}

template <typename T, int Cols>
void matx(py::module &m, const std::string &prefix, const std::string &name,
          const std::string &postfix) {
    matxx<T, Cols, 2>(m, prefix, name, postfix);
    matxx<T, Cols, 3>(m, prefix, name, postfix);
    matxx<T, Cols, 4>(m, prefix, name, postfix);
}

template <typename T>
void mat(py::module &m, const std::string &prefix, const std::string &name = "mat",
         const std::string &postfix = "") {
    matx<T, 2>(m, prefix, name, postfix);
    matx<T, 3>(m, prefix, name, postfix);
    matx<T, 4>(m, prefix, name, postfix);
}

void exposeGLMMatTypes(py::module &glmModule) {
    mat<float>(glmModule, "");
    mat<double>(glmModule, "d");
    mat<int>(glmModule, "i");
    mat<unsigned int>(glmModule, "u");
}
}  // namespace inviwo

#include <warn/pop>
