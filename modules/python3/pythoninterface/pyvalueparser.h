/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_PYVALUEPARSER_H
#define IVW_PYVALUEPARSER_H

#include <modules/python3/python3moduledefine.h>
#include <inviwo/core/util/glm.h>
#include <modules/python3/pythonincluder.h>
#include <inviwo/core/util/stringconversion.h>


namespace inviwo {
    class Property;

    class IVW_MODULE_PYTHON3_API PyValueParser {
        PyValueParser() {}
        virtual ~PyValueParser() {}
    public:

        template <typename T> static T parse(PyObject* arg);
        template <typename T> static PyObject* toPyObject(T t);
        static void setProperty(Property* p, PyObject* args);
        static PyObject* getProperty(Property* p);

        template<typename T> static bool is(PyObject* arg);

    };

    template <typename T> 
    T PyValueParser::parse(PyObject* arg) {
        return detail::parse<T>(arg);
    }

    template <> IVW_MODULE_PYTHON3_API bool        PyValueParser::parse(PyObject* args);
    template <> IVW_MODULE_PYTHON3_API std::string PyValueParser::parse(PyObject* args);

    template<typename T>
    bool PyValueParser::is(PyObject* arg) {
         return detail::test<T>(arg);
    }

    template <> IVW_MODULE_PYTHON3_API PyObject* PyValueParser::toPyObject<bool>(bool arg);
    template <> IVW_MODULE_PYTHON3_API PyObject*  PyValueParser::toPyObject<double>(double arg);
    template <> IVW_MODULE_PYTHON3_API PyObject*  PyValueParser::toPyObject<float>(float arg);
    template <> IVW_MODULE_PYTHON3_API PyObject*  PyValueParser::toPyObject<char>(char arg);
    template <> IVW_MODULE_PYTHON3_API PyObject*  PyValueParser::toPyObject<short>(short arg);
    template <> IVW_MODULE_PYTHON3_API PyObject*  PyValueParser::toPyObject<int>(int arg);
    template <> IVW_MODULE_PYTHON3_API PyObject*  PyValueParser::toPyObject<unsigned int>(unsigned int arg);
    template <> IVW_MODULE_PYTHON3_API PyObject*  PyValueParser::toPyObject<long>(long arg);
    template <> IVW_MODULE_PYTHON3_API PyObject*  PyValueParser::toPyObject<unsigned long>(unsigned long arg);
    template <> IVW_MODULE_PYTHON3_API PyObject*  PyValueParser::toPyObject<unsigned long long>(unsigned long long arg);
    template <> IVW_MODULE_PYTHON3_API PyObject*  PyValueParser::toPyObject<vec2>(vec2 arg);
    template <> IVW_MODULE_PYTHON3_API PyObject*  PyValueParser::toPyObject<vec3>(vec3 arg);
    template <> IVW_MODULE_PYTHON3_API PyObject*  PyValueParser::toPyObject<vec4>(vec4 arg);
    template <> IVW_MODULE_PYTHON3_API PyObject*  PyValueParser::toPyObject<ivec2>(ivec2 arg);
    template <> IVW_MODULE_PYTHON3_API PyObject*  PyValueParser::toPyObject<ivec3>(ivec3 arg);
    template <> IVW_MODULE_PYTHON3_API PyObject*  PyValueParser::toPyObject<ivec4>(ivec4 arg);
    template <> IVW_MODULE_PYTHON3_API PyObject*  PyValueParser::toPyObject<uvec2>(uvec2 arg);
    template <> IVW_MODULE_PYTHON3_API PyObject*  PyValueParser::toPyObject<uvec3>(uvec3 arg);
    template <> IVW_MODULE_PYTHON3_API PyObject*  PyValueParser::toPyObject<uvec4>(uvec4 arg);
    template <> IVW_MODULE_PYTHON3_API PyObject*  PyValueParser::toPyObject<mat2>(mat2 arg);
    template <> IVW_MODULE_PYTHON3_API PyObject*  PyValueParser::toPyObject<mat3>(mat3 arg);
    template <> IVW_MODULE_PYTHON3_API PyObject*  PyValueParser::toPyObject<mat4>(mat4 arg);
    template <> IVW_MODULE_PYTHON3_API PyObject*  PyValueParser::toPyObject<std::string>(std::string arg);



namespace detail {
    
template <typename T>
T parse(PyObject* args) {
    T val{};
    parseImpl<T, util::flat_extent<T>::value>(args, val);
    return val;
}

template <typename T, int N, typename... Args>
typename std::enable_if<0 < N, void>::type parseImpl(PyObject* arg, T& val, Args*... args) {
    parseImpl<T, N - 1>(arg, val, &util::glmcomp(val, N-1), args...);
}
template <typename T, int N, typename... Args>
typename std::enable_if<0 == N, void>::type parseImpl(PyObject* arg, T& val, Args*... args) {
    std::string ts = typestring<T>();
    PyArg_ParseTuple(arg, ts.c_str(), args...);
}


template <typename T, typename std::enable_if<util::rank<T>::value == 0, int>::type = 0>
std::string typestring() {
    return toString(typeToChar<T>::value);
}
template <typename T, typename std::enable_if<util::rank<T>::value == 1, int>::type = 0>
std::string typestring() {
    using type = typename T::value_type;
    std::stringstream ss;
    for (int i = 0; i < util::extent<T, 0>::value; ++i) ss << typeToChar<type>::value;
    return ss.str();
}
template <typename T, typename std::enable_if<util::rank<T>::value == 2, int>::type = 0>
std::string typestring() {
    using type = typename T::col_type;
    std::stringstream ss;
    for (int i = 0; i < util::extent<T, 1>::value; ++i) {
        ss << "(" << typestring<type>() << ")";
    }
    return ss.str();
}


template <typename T>
struct typeToChar {};

template <> struct typeToChar<unsigned char> : std::integral_constant<char, 'b'> {};
template <> struct typeToChar<short> : std::integral_constant<char, 'h'> {};
template <> struct typeToChar<unsigned short> : std::integral_constant<char, 'H'> {};
template <> struct typeToChar<int> : std::integral_constant<char, 'i'> {};
template <> struct typeToChar<unsigned int> : std::integral_constant<char, 'I'> {};
template <> struct typeToChar<long int> : std::integral_constant<char, 'l'> {};
template <> struct typeToChar<unsigned long> : std::integral_constant<char, 'k'> {};
template <> struct typeToChar<long long> : std::integral_constant<char, 'L'> {};
template <> struct typeToChar<unsigned long long> : std::integral_constant<char, 'K'> {};
template <> struct typeToChar<float> : std::integral_constant<char, 'f'> {};
template <> struct typeToChar<double> : std::integral_constant<char, 'd'> {};



template <typename T, typename std::enable_if<util::rank<T>::value == 0 &&
                                                  std::is_same<std::string, typename std::remove_cv<T>::type >::value,
                                              int>::type = 0>
static bool test(PyObject* arg) {
    return PyUnicode_Check(arg);
}

template <typename T, typename std::enable_if<util::rank<T>::value == 0 &&
                                                  std::is_same<bool, typename std::remove_cv<T>::type >::value,
                                              int>::type = 0>
static bool test(PyObject* arg) {
    return PyBool_Check(arg);
}

template <typename T, typename std::enable_if<util::rank<T>::value == 0 && std::is_integral<T>::value,
                                              int>::type = 0>
static bool test(PyObject* arg) {
    return PyLong_Check(arg);
}

template <typename T, typename std::enable_if<
                          util::rank<T>::value == 0 && util::is_floating_point<T>::value, int>::type = 0>
static bool test(PyObject* arg) {
    return PyFloat_Check(arg) || PyLong_Check(arg);
}

template <typename T, typename std::enable_if<util::rank<T>::value == 1, int>::type = 0>
static bool test(PyObject* arg) {
    if (!PyTuple_Check(arg)) return false;

    if (PyTuple_Size(arg) != util::extent<T>::value) return false;

    for (size_t i = 0; i < util::extent<T>::value; i++) {
        if (!test<typename T::value_type>(PyTuple_GetItem(arg, i))) return false;
    }
    return true;
}

template <typename T, typename std::enable_if<util::rank<T>::value == 2, int>::type = 0>
static bool test(PyObject* arg) {
    if (!PyTuple_Check(arg)) return false;

    if (PyTuple_Size(arg) != util::extent<T>::value) return false;

    for (size_t i = 0; i < util::extent<T>::value; i++) {
        if (!test<typename T::col_type>(PyTuple_GetItem(arg, i))) return false;
    }
    return true;
}


} // detail

} // namespace

#endif // IVW_PYVALUEPARSER_H

