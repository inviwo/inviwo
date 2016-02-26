/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#ifndef IVW_PYTHONPARAMETERPARSER_H
#define IVW_PYTHONPARAMETERPARSER_H

#include <modules/python3/python3moduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/python3/pythoninterface/pyvalueparser.h>

#include <modules/python3/pythonincluder.h>

namespace inviwo {

/**
 * \class PythonParameterParser
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS
 */
class IVW_MODULE_PYTHON3_API PythonParameterParser { 
public:
        PythonParameterParser(size_t optional = 0);


        template <typename... Types>
        int parse(PyObject* args, Types&... outputs);
        
        int parse(PyObject* args);

    private:
        template <typename T>
        std::string typeToString();

        template <typename T, typename... Types2>
        int parseArray(int idx, PyObject* args, T& t, Types2&... outputs);

        template <typename T>
        int parseArray(int idx, PyObject* args, T& t);

        template <typename T>
        bool parseValue(int idx, PyObject* obj, T& t);

        bool parseValue(int idx, PyObject* obj, PyObject*& t);

        size_t optional_;
    };


template <typename T>
std::string PythonParameterParser::typeToString()
{
    return parseTypeIdName(typeid(T).name());
}

template <> IVW_MODULE_PYTHON3_API
std::string PythonParameterParser::typeToString<std::string>();

template <typename... Types>
int PythonParameterParser::parse(PyObject* args, Types&... outputs) {
    auto params = static_cast<size_t>(PyTuple_Size(args));
    auto needed = sizeof...(Types);
    auto nonOptional = needed - optional_;

    if (params < nonOptional) {
        std::stringstream ss;
        ss << "Function expects " << needed << " parameters ";
        if (optional_ != 0) {
            ss << "(" << optional_ << " optional) ";
        }
        ss << "got " << params;

        PyErr_SetString(PyExc_TypeError, ss.str().c_str());
        return -1;
    }

    auto v = parseArray(0, args, outputs...);
    return v;
}


template <typename T, typename... Types2>
int PythonParameterParser::parseArray(int idx, PyObject* args, T& t, Types2&... outputs) {
    auto params = PyTuple_Size(args);
    if (idx >= params) return 0;
    if (!parseValue(idx, PyTuple_GetItem(args, idx), t)) {
        return -1;
    }
    auto v = parseArray(idx + 1, args, outputs...);
    return v == -1 ? -1 : 1 + v;
}


template <typename T>
int PythonParameterParser::parseArray(int idx, PyObject* args, T& t) {
    auto params = PyTuple_Size(args);
    if (idx >= params) return 0;
    return parseValue(idx, PyTuple_GetItem(args, idx), t);
}


template <typename T>
bool PythonParameterParser::parseValue(int idx, PyObject* obj, T& t) {
    if (!obj) return false;
    if (PyValueParser::is<T>(obj)) {
        t = PyValueParser::parse<T>(obj);
        return true;
    }

    auto tt = dynamic_cast<PyTypeObject*>(obj->ob_type);

    std::stringstream ss;
    ss << "function expects parameter " << (idx + 1) << " to be " << typeToString<T>()
        << ", got: " << tt->tp_name;
    // TODO print better error message
    // typeid is not good!! gives: std::basic_string<char,struct std::char_traits<char>,class
    // std::allocator<char> >
    // for string
    PyErr_SetString(PyExc_TypeError, ss.str().c_str());

    return false;
}



} // namespace

#endif // IVW_PYTHONPARAMETERPARSER_H

