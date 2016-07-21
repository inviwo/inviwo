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

#ifndef IVW_PYVALUEPARSER_H
#define IVW_PYVALUEPARSER_H

#include <modules/python3/python3moduledefine.h>
#include <inviwo/core/util/glm.h>
#include <modules/python3/pythonincluder.h>
#include <inviwo/core/util/stringconversion.h>

#include <tuple>
#include <utility>

namespace inviwo {
class Property;

class IVW_MODULE_PYTHON3_API PyValueParser {
    PyValueParser() {}
    virtual ~PyValueParser() {}

public:
    template <typename T>
    static T parse(PyObject* arg);
    
    template <typename T>
    static PyObject* toPyObject(T t);
    
    template <typename T>
    static bool is(PyObject* arg);
    
    static bool setProperty(Property* p, PyObject* args);
    static PyObject* getProperty(Property* p);
};

namespace detail {

template<size_t...>
struct IntSequence { };

template<size_t N, size_t... S>
struct GenerateIntSequence : GenerateIntSequence<N-1, N-1, S...> { };

template<size_t... S>
struct GenerateIntSequence<0, S...> {
  typedef IntSequence<S...> type;
};

template <size_t N>
using GenerateIntSequence_t = typename GenerateIntSequence<N>::type;


template<typename Dependent, size_t Index>
using DependOn = Dependent;

template<typename T, size_t N, typename I = GenerateIntSequence_t<N>>
struct repeat;

template<typename T, size_t N, size_t... Indices>
struct repeat<T, N, IntSequence<Indices...>> {
    using type = std::tuple<DependOn<T, Indices>...>;
};



template <typename T>
using glmToTupleType = typename repeat<typename T::value_type, util::flat_extent<T>::value>::type;

template <typename T, int N, typename... Args>
typename std::enable_if<0 == N, glmToTupleType<T>>::type glmToFlatTupleImpl(const T& val,
                                                                            Args... args) {
    return std::make_tuple(args...);
}
template <typename T, int N, typename... Args>
typename std::enable_if<0 < N, glmToTupleType<T>>::type glmToFlatTupleImpl(const T& val,
                                                                           Args... args) {
    return glmToFlatTupleImpl<T, N - 1>(val, util::glmcomp(val, N - 1), args...);
}
template <typename T>
auto glmToFlatTuple(const T& arg) -> glmToTupleType<T> {
    return glmToFlatTupleImpl<T, util::flat_extent<T>::value>(arg);
}

template <typename T>
struct typeToPy {
    static std::string str() {
        using type = typename T::value_type;
        std::stringstream ss;

        if (util::rank<T>::value == 1) {
            for (size_t i = 0; i < util::extent<T, 0>::value; ++i) {
                ss << typeToPy<type>::str();
            }

        } else if (util::rank<T>::value == 2) {
            for (size_t i = 0; i < util::extent<T, 0>::value; ++i) {
                ss << "(";
                for (size_t j = 0; j < util::extent<T, 1>::value; ++j) {
                    ss << typeToPy<type>::str();
                }
                ss << ")";
            }
        }
        return ss.str();
    }

    static auto data(const T& val) -> glmToTupleType<T> {
        return glmToFlatTuple<T>(val);
    }
};

template <typename U, typename V>
struct typeToPy<std::pair<U, V>> {
    static std::string str() { return "(" + typeToPy<U>::str() + typeToPy<V>::str() + ")"; }

    static auto data(const std::pair<U, V>& p)
        -> decltype(std::tuple_cat(typeToPy<U>::data(std::declval<U>()),
                                   typeToPy<V>::data(std::declval<V>()))) {
        return std::tuple_cat(typeToPy<U>::data(p.first), typeToPy<V>::data(p.second));
    }
};

template <>
struct typeToPy<unsigned char>{
    static std::string str() { return "b"; }
    static std::tuple<unsigned char> data(unsigned char v) { return std::make_tuple(v); }
};    
template <>
struct typeToPy<short>{
    static std::string str() { return "h"; }
    static std::tuple<short> data(short v) { return std::make_tuple(v); }
};    
template <>
struct typeToPy<unsigned short>{
    static std::string str() { return "H"; }
    static std::tuple<unsigned short> data(unsigned short v) { return std::make_tuple(v); }
};    
template <>
struct typeToPy<int>{
    static std::string str() { return "i"; }
    static std::tuple<int> data(int v) { return std::make_tuple(v); }
};    
template <>
struct typeToPy<unsigned int>{
    static std::string str() { return "I"; }
    static std::tuple<unsigned int> data(unsigned int v) { return std::make_tuple(v); }
};    
template <>
struct typeToPy<long int>{
    static std::string str() { return "l"; }
    static std::tuple<long int> data(long int v) { return std::make_tuple(v); }
};    
template <>
struct typeToPy<unsigned long>{
    static std::string str() { return "k"; }
    static std::tuple<unsigned long> data(unsigned long v) { return std::make_tuple(v); }
};    
template <>
struct typeToPy<long long>{
    static std::string str() { return "L"; }
    static std::tuple<long long> data(long long v) { return std::make_tuple(v); }
};    
template <>
struct typeToPy<unsigned long long>{
    static std::string str() { return "K"; }
    static std::tuple<unsigned long long> data(unsigned long long v) { return std::make_tuple(v); }
};    
template <>
struct typeToPy<float>{
    static std::string str() { return "f"; }
    static std::tuple<float> data(float v) { return std::make_tuple(v); }
};    
template <>
struct typeToPy<double>{
    static std::string str() { return "d"; }
    static std::tuple<double> data(double v) { return std::make_tuple(v); }
};
template <>
struct typeToPy<std::string> {
    static std::string str() { return "s#"; }
    static auto data(const std::string& f)
        -> decltype(std::make_tuple(std::declval<std::string>().c_str(),
                                    std::declval<std::string>().size())) {
        return std::make_tuple(f.c_str(), f.size());
    }
};



// Parse: PyObject -> C++
template <typename T, int N, typename... Args>
typename std::enable_if<0 == N, void>::type parseImpl(PyObject* arg, T& val, Args*... args) {
    std::string ts = typeToPy<T>::str();
    if (PyTuple_Check(arg)) 
        PyArg_ParseTuple(arg, ts.c_str(), args...);
    else
        PyArg_Parse(arg, ts.c_str(), args...);
}
template <typename T, int N, typename... Args>
typename std::enable_if<0 < N, void>::type parseImpl(PyObject* arg, T& val, Args*... args) {
    parseImpl<T, N - 1>(arg, val, &util::glmcomp(val, N - 1), args...);
}
template <typename T>
T parse(PyObject* args) {
    T val{};
    parseImpl<T, util::flat_extent<T>::value>(args, val);
    return val;
}


// Build C++ -> PyObject

template <typename T, size_t... S>
PyObject* buildHelper(const std::string& ts, const T& params, IntSequence<S...>) {
    return Py_BuildValue(ts.c_str(), std::get<S>(params)...);;
}

template <typename T>
PyObject* build(const T& arg) {
    auto str = typeToPy<T>::str();
    auto vals = typeToPy<T>::data(arg);
    return buildHelper(
        str, vals, GenerateIntSequence_t<std::tuple_size<decltype(vals)>::value>());
}

// Test, check PyObject
template <typename T, typename std::enable_if<
                          util::rank<T>::value == 0 &&
                              std::is_same<std::string, typename std::remove_cv<T>::type>::value,
                          int>::type = 0>
static bool test(PyObject* arg) {
    return PyUnicode_Check(arg);
}

template <typename T,
          typename std::enable_if<util::rank<T>::value == 0 &&
                                      std::is_same<bool, typename std::remove_cv<T>::type>::value,
                                  int>::type = 0>
static bool test(PyObject* arg) {
    return PyBool_Check(arg);
}

template <typename T, typename std::enable_if<
                          util::rank<T>::value == 0 &&
                          !(std::is_same<bool, typename std::remove_cv<T>::type>::value) &&
                          std::is_integral<T>::value,
                          int>::type = 0>
static bool test(PyObject* arg) {
    return PyLong_Check(arg);
}

template <typename T,
          typename std::enable_if<util::rank<T>::value == 0 && util::is_floating_point<T>::value,
                                  int>::type = 0>
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

}  // detail

template <typename T>
T PyValueParser::parse(PyObject* arg) {
    return detail::parse<T>(arg);
}
template <> IVW_MODULE_PYTHON3_API bool PyValueParser::parse(PyObject* args);
template <> IVW_MODULE_PYTHON3_API std::string PyValueParser::parse(PyObject* args);

template <typename T>
PyObject* PyValueParser::toPyObject(T arg) {
    return detail::build<T>(arg);
}
template <>
IVW_MODULE_PYTHON3_API PyObject* PyValueParser::toPyObject<bool>(bool arg);
template <>
IVW_MODULE_PYTHON3_API PyObject* PyValueParser::toPyObject<std::string>(std::string arg);


template <typename T>
bool PyValueParser::is(PyObject* arg) {
    if (!arg) return false;
    return detail::test<T>(arg);
}

template <> IVW_MODULE_PYTHON3_API
bool PyValueParser::is<bool>(PyObject* arg);

}  // namespace

#endif  // IVW_PYVALUEPARSER_H
