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
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <inviwo/core/util/stringconversion.h>

#include <map>
#include <string>
#include <inviwo/core/util/logcentral.h>

#include <pybind11/operators.h>

namespace py = pybind11;


template <typename T, typename V, unsigned C, typename std::enable_if<C == 0, int>::type = 0> void addInit(py::class_<V> &pyv) { pyv.def(py::init<>()); }
template <typename T, typename V, unsigned C, typename std::enable_if<C == 1, int>::type = 0> void addInit(py::class_<V> &pyv) { pyv.def(py::init<T>()); }
template <typename T, typename V, unsigned C, typename std::enable_if<C == 2, int>::type = 0> void addInit(py::class_<V> &pyv) { pyv.def(py::init<T, T>()); }
template <typename T, typename V, unsigned C, typename std::enable_if<C == 3, int>::type = 0> void addInit(py::class_<V> &pyv) { pyv.def(py::init<T, T, T>()); }
template <typename T, typename V, unsigned C, typename std::enable_if<C == 4, int>::type = 0> void addInit(py::class_<V> &pyv) { pyv.def(py::init<T, T, T, T>()); }
template <typename T, typename V, unsigned C, typename std::enable_if<C == 5, int>::type = 0> void addInit(py::class_<V> &pyv) { pyv.def(py::init<T, T, T, T, T>()); }
template <typename T, typename V, unsigned C, typename std::enable_if<C == 6, int>::type = 0> void addInit(py::class_<V> &pyv) { pyv.def(py::init<T, T, T, T, T, T>()); }
template <typename T, typename V, unsigned C, typename std::enable_if<C == 7, int>::type = 0> void addInit(py::class_<V> &pyv) { pyv.def(py::init<T, T, T, T, T, T, T>()); }
template <typename T, typename V, unsigned C, typename std::enable_if<C == 8, int>::type = 0> void addInit(py::class_<V> &pyv) { pyv.def(py::init<T, T, T, T, T, T, T, T>()); }
template <typename T, typename V, unsigned C, typename std::enable_if<C == 9, int>::type = 0> void addInit(py::class_<V> &pyv) { pyv.def(py::init<T, T, T, T, T, T, T, T, T>()); }
template <typename T, typename V, unsigned C, typename std::enable_if<C == 10, int>::type = 0> void addInit(py::class_<V> &pyv) { pyv.def(py::init<T, T, T, T, T, T, T, T, T, T>()); }
template <typename T, typename V, unsigned C, typename std::enable_if<C == 11, int>::type = 0> void addInit(py::class_<V> &pyv) { pyv.def(py::init<T, T, T, T, T, T, T, T, T, T, T>()); }
template <typename T, typename V, unsigned C, typename std::enable_if<C == 12, int>::type = 0> void addInit(py::class_<V> &pyv) { pyv.def(py::init<T, T, T, T, T, T, T, T, T, T, T, T>()); }
template <typename T, typename V, unsigned C, typename std::enable_if<C == 13, int>::type = 0> void addInit(py::class_<V> &pyv) { pyv.def(py::init<T, T, T, T, T, T, T, T, T, T, T, T, T>()); }
template <typename T, typename V, unsigned C, typename std::enable_if<C == 14, int>::type = 0> void addInit(py::class_<V> &pyv) { pyv.def(py::init<T, T, T, T, T, T, T, T, T, T, T, T, T, T>()); }
template <typename T, typename V, unsigned C, typename std::enable_if<C == 15, int>::type = 0> void addInit(py::class_<V> &pyv) { pyv.def(py::init<T, T, T, T, T, T, T, T, T, T, T, T, T, T, T>()); }
template <typename T, typename V, unsigned C, typename std::enable_if<C == 16, int>::type = 0> void addInit(py::class_<V> &pyv) { pyv.def(py::init<T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T>()); }

namespace inviwo {
   


    template <typename T,unsigned A>
    void vecx(py::module &m, std::string prefix, std::string name = "vec", std::string postfix = "") {
        using V = Vector<A, T>;
        std::stringstream classname;
        classname << prefix << name << A << postfix;
        py::class_<V> pyv(m, classname.str().c_str());
        pyv
            .def(py::init<T>())
            .def(py::init<>())
            .def("__repr__", [](V &v) { return glm::to_string(v); })
            .def(py::self + py::self)
            .def(py::self - py::self)
            .def(py::self * py::self)
            .def(py::self / py::self)
            .def(py::self += py::self)
            .def(py::self -= py::self)
            .def(py::self *= py::self)
            .def(py::self /= py::self)
            .def(py::self == py::self)
            .def(py::self != py::self)
            //.def(py::self < py::self)
            //.def(py::self > py::self)
            //.def(py::self >= py::self)
            //.def(py::self <= py::self)
            ;

        addInit<T, V, A>(pyv);
    }

    template <typename T>
    void vec(py::module &m, std::string prefix , std::string name = "vec" , std::string postfix = "") {
        vecx<T, 2>(m, prefix, name, postfix);
        vecx<T, 3>(m, prefix, name, postfix);
        vecx<T, 4>(m, prefix, name, postfix);
    }


    template <typename T, unsigned COLS, unsigned ROWS>
    void matxx(py::module &m, std::string prefix, std::string name = "mat", std::string postfix = "") {
        using M = typename util::glmtype<T, COLS, ROWS>::type;
        using Va = Vector<COLS, T>;
        using Vb = Vector<ROWS, T>;

        std::stringstream classname;
        classname << prefix << name << COLS;
        if (COLS != ROWS) {
            classname << "x" << ROWS;
        }

        M masdf(glm::vec2(), glm::vec2(), glm::vec2());
        M masdf2(glm::vec3(), glm::vec3());

        py::class_<M> pym(m, classname.str().c_str());
        pym
            .def(py::init<T>())
            .def(py::init<>())
            .def("__repr__", [](M &m) { return glm::to_string(m); })
            .def(py::self + py::self)
            .def(py::self - py::self)
//            .def(py::self * py::self)
//            .def(py::self / py::self)
            .def(py::self == py::self)
            .def(py::self != py::self)
            //.def(py::self < py::self)
            //.def(py::self > py::self)
            //.def(py::self >= py::self)
            //.def(py::self <= py::self)
            ;

        addInit<T, M, COLS*ROWS>(pym);
        addInit<typename M::col_type, M, ROWS>(pym);
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

    void addGLMTypes(py::module &m) {
        auto glmModule = m.def_submodule("glm", "Exposing glm vec and mat types");

        glmtypes<float>(glmModule, "");
        glmtypes<double>(glmModule, "d");
        glmtypes<int>(glmModule, "i");
        glmtypes<glm::uint64>(glmModule, "u64");
        glmtypes<unsigned int>(glmModule, "u");
    //    glmtypes<size_t>(glmModule, "" , "size" , "_t");

    }
}