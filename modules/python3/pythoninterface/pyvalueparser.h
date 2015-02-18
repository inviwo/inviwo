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
#include <inviwo/core/common/inviwo.h>

#ifndef PyObject_HEAD
struct PyObject;
struct PyMethodDef;
typedef PyObject *(*PyCFunction)(PyObject *, PyObject *);
#endif

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


    template <> IVW_MODULE_PYTHON3_API bool        PyValueParser::parse(PyObject* args);
    template <> IVW_MODULE_PYTHON3_API double      PyValueParser::parse(PyObject* args);
    template <> IVW_MODULE_PYTHON3_API float       PyValueParser::parse(PyObject* args);
    template <> IVW_MODULE_PYTHON3_API char        PyValueParser::parse(PyObject* args);
    template <> IVW_MODULE_PYTHON3_API short       PyValueParser::parse(PyObject* args);
    template <> IVW_MODULE_PYTHON3_API int         PyValueParser::parse(PyObject* args);
    template <> IVW_MODULE_PYTHON3_API unsigned int PyValueParser::parse(PyObject* args);
    template <> IVW_MODULE_PYTHON3_API long        PyValueParser::parse(PyObject* args);
    template <> IVW_MODULE_PYTHON3_API unsigned long PyValueParser::parse(PyObject* args);
    template <> IVW_MODULE_PYTHON3_API unsigned long long PyValueParser::parse(PyObject* args);
    template <> IVW_MODULE_PYTHON3_API vec2        PyValueParser::parse(PyObject* args);
    template <> IVW_MODULE_PYTHON3_API vec3        PyValueParser::parse(PyObject* args);
    template <> IVW_MODULE_PYTHON3_API vec4        PyValueParser::parse(PyObject* args);
    template <> IVW_MODULE_PYTHON3_API ivec2       PyValueParser::parse(PyObject* args);
    template <> IVW_MODULE_PYTHON3_API ivec3       PyValueParser::parse(PyObject* args);
    template <> IVW_MODULE_PYTHON3_API ivec4       PyValueParser::parse(PyObject* args);
    template <> IVW_MODULE_PYTHON3_API uvec2       PyValueParser::parse(PyObject* args);
    template <> IVW_MODULE_PYTHON3_API uvec3       PyValueParser::parse(PyObject* args);
    template <> IVW_MODULE_PYTHON3_API uvec4       PyValueParser::parse(PyObject* args);
    template <> IVW_MODULE_PYTHON3_API mat2        PyValueParser::parse(PyObject* args);
    template <> IVW_MODULE_PYTHON3_API mat3        PyValueParser::parse(PyObject* args);
    template <> IVW_MODULE_PYTHON3_API mat4        PyValueParser::parse(PyObject* args);
    template <> IVW_MODULE_PYTHON3_API std::string PyValueParser::parse(PyObject* args);


    template <> IVW_MODULE_PYTHON3_API bool PyValueParser::is<bool>(PyObject* arg);
    template <> IVW_MODULE_PYTHON3_API bool PyValueParser::is<double>(PyObject* arg);
    template <> IVW_MODULE_PYTHON3_API bool PyValueParser::is<float>(PyObject* arg);
    template <> IVW_MODULE_PYTHON3_API bool PyValueParser::is<char>(PyObject* arg);
    template <> IVW_MODULE_PYTHON3_API bool PyValueParser::is<short>(PyObject* arg);
    template <> IVW_MODULE_PYTHON3_API bool PyValueParser::is<int>(PyObject* arg);
    template <> IVW_MODULE_PYTHON3_API bool PyValueParser::is<unsigned int>(PyObject* arg);
    template <> IVW_MODULE_PYTHON3_API bool PyValueParser::is<long>(PyObject* arg);
    template <> IVW_MODULE_PYTHON3_API bool PyValueParser::is<unsigned long>(PyObject* arg);
    template <> IVW_MODULE_PYTHON3_API bool PyValueParser::is<unsigned long long>(PyObject* arg);
    template <> IVW_MODULE_PYTHON3_API bool PyValueParser::is<vec2>(PyObject* arg);
    template <> IVW_MODULE_PYTHON3_API bool PyValueParser::is<vec3>(PyObject* arg);
    template <> IVW_MODULE_PYTHON3_API bool PyValueParser::is<vec4>(PyObject* arg);
    template <> IVW_MODULE_PYTHON3_API bool PyValueParser::is<ivec2>(PyObject* arg);
    template <> IVW_MODULE_PYTHON3_API bool PyValueParser::is<ivec3>(PyObject* arg);
    template <> IVW_MODULE_PYTHON3_API bool PyValueParser::is<ivec4>(PyObject* arg);
    template <> IVW_MODULE_PYTHON3_API bool PyValueParser::is<uvec2>(PyObject* arg);
    template <> IVW_MODULE_PYTHON3_API bool PyValueParser::is<uvec3>(PyObject* arg);
    template <> IVW_MODULE_PYTHON3_API bool PyValueParser::is<uvec4>(PyObject* arg);
    template <> IVW_MODULE_PYTHON3_API bool PyValueParser::is<mat2>(PyObject* arg);
    template <> IVW_MODULE_PYTHON3_API bool PyValueParser::is<mat3>(PyObject* arg);
    template <> IVW_MODULE_PYTHON3_API bool PyValueParser::is<mat4>(PyObject* arg);
    template <> IVW_MODULE_PYTHON3_API bool PyValueParser::is<std::string>(PyObject* arg);

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



} // namespace

#endif // IVW_PYVALUEPARSER_H

