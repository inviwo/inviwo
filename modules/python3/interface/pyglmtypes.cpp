/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2016 Inviwo Foundation
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

#include <modules/python3/interface/pyglmtypes.h>

#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/logcentral.h>

#include <pybind11/operators.h>
#include <pybind11/numpy.h>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <map>
#include <string>
#include <algorithm>

namespace py = pybind11;

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

namespace inviwo {

template <typename T, typename GLM>
void common(py::module &m, py::class_<GLM> &pyc, std::string name) {
    pyc.def(py::init<T>())
        .def(py::init<>())
        .def("__repr__", [](GLM &v) { return glm::to_string(v); })

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

        .def("__getitem__", [](GLM &v, int idx) { return &v[idx]; },
             py::return_value_policy::reference_internal);

    py::bind_vector<std::vector<GLM>>(m, name + "Vector");
}

template <typename T, typename V>
void floatOnlyVecs(py::module &m, std::false_type) {}

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

template <typename T, unsigned A>
void vecx(py::module &m, std::string prefix, std::string name = "vec", std::string postfix = "") {
    using V = Vector<A, T>;
    std::stringstream classname;
    classname << prefix << name << A << postfix;
    py::class_<V> pyv(m, classname.str().c_str());
    common<T>(m, pyv, classname.str());
    addInit<T, V, A>(pyv);
    pyv.def(py::self * py::self)
        .def(py::self / py::self)
        .def(py::self *= py::self)
        .def(py::self /= py::self)

        .def_property_readonly("nparray",[](V &self){ return py::array_t<T>(A,glm::value_ptr(self));})

        .def("__str__",
             [](V &v) {
                 std::ostringstream oss;
                 oss << "[";
                 for (int i = 0; i < A; i++) {
                     if (i != 0) {
                         oss << " ";
                     }
                     oss << v[i];
                 }
                 oss << "]";
                 return oss.str();
             })

        .def("__setitem__", [](V &v, int idx, T &t) { return v[idx] = t; });
    floatOnlyVecs<T, V>(m);

    switch (A) {
        case 4:
            pyv.def_property("w", [](V &b) { return b[3]; }, [](V &b, T t) { b[3] = t; });
            pyv.def_property("a", [](V &b) { return b[3]; }, [](V &b, T t) { b[3] = t; });
            pyv.def_property("q", [](V &b) { return b[3]; }, [](V &b, T t) { b[3] = t; });
        case 3:
            pyv.def_property("z", [](V &b) { return b[2]; }, [](V &b, T t) { b[2] = t; });
            pyv.def_property("b", [](V &b) { return b[2]; }, [](V &b, T t) { b[2] = t; });
            pyv.def_property("p", [](V &b) { return b[2]; }, [](V &b, T t) { b[2] = t; });
        case 2:
            pyv.def_property("y", [](V &b) { return b[1]; }, [](V &b, T t) { b[1] = t; });
            pyv.def_property("g", [](V &b) { return b[1]; }, [](V &b, T t) { b[1] = t; });
            pyv.def_property("t", [](V &b) { return b[1]; }, [](V &b, T t) { b[1] = t; });
            pyv.def_property("x", [](V &b) { return b[0]; }, [](V &b, T t) { b[0] = t; });
            pyv.def_property("r", [](V &b) { return b[0]; }, [](V &b, T t) { b[0] = t; });
            pyv.def_property("s", [](V &b) { return b[0]; }, [](V &b, T t) { b[0] = t; });
        default:
            break;
    }
}

template <typename T>
void vec(py::module &m, std::string prefix, std::string name = "vec", std::string postfix = "") {
    vecx<T, 2>(m, prefix, name, postfix);
    vecx<T, 3>(m, prefix, name, postfix);
    vecx<T, 4>(m, prefix, name, postfix);
}

template <typename T, unsigned COLS, unsigned ROWS>
void matxx(py::module &m, std::string prefix, std::string name = "mat", std::string postfix = "") {
    
    using M = typename util::glmtype<T, COLS, ROWS >::type;

    using ColumnVector = typename M::col_type;
    using RowVector = typename M::row_type;

    using Ma2 = typename util::glmtype<T, 2, COLS>::type; 
    using Ma3 = typename util::glmtype<T, 3, COLS>::type;
    using Ma4 = typename util::glmtype<T, 4, COLS>::type;

    std::stringstream classname;
    classname << prefix << name;
    if (COLS != ROWS) {
        classname << COLS << "x" << ROWS;
    }else{
        classname << COLS;
    }

    py::class_<M> pym(m, classname.str().c_str());
    common<T>(m, pym, classname.str());
    addInit<T, M, COLS * ROWS>(pym);
    addInit<typename M::col_type, M, COLS>(pym);
    pym
        .def(py::self * RowVector())
        .def(ColumnVector() * py::self)
        .def(py::self * Ma2())
        .def(py::self * Ma3())
        .def(py::self * Ma4())

        .def_property_readonly("nparray", [](M &self) { return py::array_t<T>(std::vector<size_t>{ROWS,COLS}, glm::value_ptr(self)); })

        .def("__getitem__", [](M &m, int idx, int idy) { return m[idx][idy]; })
        .def("__setitem__", [](M &m, int idx, ColumnVector &t) { return m[idx] = t; })
        .def("__setitem__", [](M &m, int idx, int idy, T &t) { return m[idx][idy] = t; })

        .def("__str__",
             [](M &m) {
                 std::ostringstream oss;
                 oss << "[";
                 for (int col = 0; col < COLS; col++) {
                     if (col != 0) {
                         oss << ",";
                     }
                     oss << "[";
                     for (int row = 0; row < ROWS; row++) {
                         if (row != 0) {
                             oss << " ";
                         }
                         oss << m[col][row];
                     }
                     oss << "]";
                 }
                 oss << "]";
                 return oss.str();
             })
        ;
}

template <typename T, unsigned COLS>
void matx(py::module &m, std::string prefix, std::string name = "mat", std::string postfix = "") {
    matxx<T, COLS, 2>(m, prefix, name, postfix);
    matxx<T, COLS, 3>(m, prefix, name, postfix);
    matxx<T, COLS, 4>(m, prefix, name, postfix);
}

template <typename T>
void mat(py::module &m, std::string prefix, std::string name = "mat", std::string postfix = "") {
    matx<T, 2>(m, prefix, name, postfix);
    matx<T, 3>(m, prefix, name, postfix);
    matx<T, 4>(m, prefix, name, postfix);
}

template <typename T>
void glmtypes(py::module &m, std::string prefix, std::string postfix = "") {
    vec<T>(m, prefix, "vec", postfix);
    mat<T>(m, prefix, "mat", postfix);
}

template <typename B>
struct A : public B {};

void exposeGLMTypes(py::module &m) {
    auto glmModule = m.def_submodule("glm", "Exposing glm vec and mat types");

    glmtypes<float>(glmModule, "");
    glmtypes<double>(glmModule, "d");
    glmtypes<int>(glmModule, "i");
    glmtypes<unsigned int>(glmModule, "u");
    vec<size_t>(glmModule, "","size", "_t");
}
}
