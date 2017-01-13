/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#ifndef IVW_PYTHON_UTILITIES_H
#define IVW_PYTHON_UTILITIES_H

#include <modules/python3/python3moduledefine.h>

#include <modules/python3/pythonincluder.h>

#include <modules/python3/pythoninterface/pythonparameterparser.h>
#include <modules/python3/pythoninterface/pyvalueparser.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

namespace utilpy {

template <std::size_t N, class T>
using tuple_element_t = typename std::tuple_element<N, T>::type;

template <typename T>
PyObject* makePyList(std::vector<T> items) {
    PyObject* list = PyList_New(0);
    for (const auto& item : items) {
        auto listitem = PyValueParser::toPyObject(item);
        if (PyList_Append(list, listitem) != 0) {
            LogInfoCustom("Python", "Error on list append");
        }
    }
    return list;
}

ProcessorMetaData* getProcessorMetaData(const std::string& identifier);

template <typename T>
PyObject* setProcessorMetaDataProperty(PyObject* args, void (ProcessorMetaData::*func)(T)) {
    static PythonParameterParser tester;
    using Type = typename std::decay<T>::type;

    std::string identifier;
    Type value;
    if (tester.parse<std::string, Type>(args, identifier, value) == 2) {
        if (auto m = utilpy::getProcessorMetaData(identifier)) {
            ((*m).*(func))(value);
            Py_RETURN_NONE;
        }
    }

    std::string msg = std::string("Could not find a processor with identifier: ") + identifier;
    PyErr_SetString(PyExc_ValueError, msg.c_str());
    return nullptr;
}

template <typename T>
PyObject* getProcessorMetaDataProperty(PyObject* args, T (ProcessorMetaData::*func)() const) {
    static PythonParameterParser tester;

    std::string identifier;
    if (tester.parse<std::string>(args, identifier) == 1) {
        if (auto m = utilpy::getProcessorMetaData(identifier)) {
            return PyValueParser::toPyObject(((*m).*(func))());
        }
    }

    std::string msg = std::string("Could not find a processor with identifier: ") + identifier;
    PyErr_SetString(PyExc_ValueError, msg.c_str());
    return nullptr;
}

template <typename Callable, typename T, size_t... S>
PyObject* setProcessorPropertyHelper(PyObject* args, Callable func, T vars, 
                                     detail::IntSequence<S...>) {
    static PythonParameterParser tester;

    std::string identifier;
    if (tester.parse<std::string, tuple_element_t<S, T>...>(
            args, identifier, std::get<S>(vars)...) == 1 + std::tuple_size<T>::value) {
        if (auto p = InviwoApplication::getPtr()->getProcessorNetwork()->getProcessorByIdentifier(
                identifier)) {
            return func(p, std::get<S>(vars)...);
        }
    }

    std::string msg = std::string("Could not find a processor with identifier: ") + identifier;
    PyErr_SetString(PyExc_ValueError, msg.c_str());
    return nullptr;
}
template <typename... Ts, typename Callable>
PyObject* setProcessorProperty(PyObject* args, Callable func) {
    return setProcessorPropertyHelper(args, func, std::tuple<Ts...>(),
                               detail::GenerateIntSequence_t<sizeof...(Ts)>());
}

template <typename Callable>
PyObject* getProcessorProperty(PyObject* args, Callable func) {
    static PythonParameterParser tester;

    std::string identifier;
    if (tester.parse<std::string>(args, identifier) == 1) {
        if (auto p = InviwoApplication::getPtr()->getProcessorNetwork()->getProcessorByIdentifier(
                identifier)) {
            return PyValueParser::toPyObject(func(p));
        }
    }

    std::string msg = std::string("Could not find a processor with identifier: ") + identifier;
    PyErr_SetString(PyExc_ValueError, msg.c_str());
    return nullptr;
}

template <typename Callable>
PyObject* getProcessorPropertyList(PyObject* args, Callable func) {
    static PythonParameterParser tester;

    std::string identifier;
    if (tester.parse<std::string>(args, identifier) == 1) {
        if (auto p = InviwoApplication::getPtr()->getProcessorNetwork()->getProcessorByIdentifier(
            identifier)) {
            auto vec = func(p);

            return utilpy::makePyList(vec);
        }
    }

    std::string msg = std::string("Could not find a processor with identifier: ") + identifier;
    PyErr_SetString(PyExc_ValueError, msg.c_str());
    return nullptr;
}

template <typename Callable, typename T, size_t... S>
PyObject* PropertyCallbackHelper(PyObject* args, Callable func, T vars, detail::IntSequence<S...>) {
    static PythonParameterParser tester;

    std::string path;
    if (tester.parse<std::string, tuple_element_t<S, T>...>(args, path, std::get<S>(vars)...) ==
        1 + std::tuple_size<T>::value) {
        if (auto p = InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(
                splitString(path, '.'))) {
            return func(p, std::get<S>(vars)...);
        }
    }

    std::string msg = std::string("Could not find a property with path: ") + path;
    PyErr_SetString(PyExc_ValueError, msg.c_str());
    return nullptr;
}

template <typename... Ts, typename Callable>
PyObject* propertyCallback(PyObject* args, Callable func) {
    return PropertyCallbackHelper(args, func, std::tuple<Ts...>(),
                                  detail::GenerateIntSequence_t<sizeof...(Ts)>());
}

}  // namespace

}  // namespace

#endif  // IVW_PYTHON_UTILITIES_H
