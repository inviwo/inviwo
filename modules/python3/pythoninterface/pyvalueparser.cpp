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

#include <modules/python3/pythonincluder.h>
#include "pyvalueparser.h"
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/baseoptionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/stringproperty.h>

namespace inviwo {

bool parseBool(PyObject* args) { return PyObject_IsTrue(args) != 0; };

std::string parseStr(PyObject* args) {
    PyObject* temp_bytes = PyUnicode_AsEncodedString(args, "ASCII", "strict");  // Owned reference
    if (temp_bytes != nullptr) {
        char* my_result = PyBytes_AS_STRING(temp_bytes);  // Borrowed pointer
        std::string str = my_result;
        Py_DECREF(temp_bytes);
        return str;
    } else {
        LogErrorCustom("PyValueParser", "TODO: Handle encoding error.");
        return "";
    }
};

template <>
bool PyValueParser::parse(PyObject* args) {
    return parseBool(args);
}
template <>
std::string PyValueParser::parse(PyObject* args) {
    return parseStr(args);
}

template <>
PyObject* PyValueParser::toPyObject(bool b) {
    return Py_BuildValue("O", b ? Py_True : Py_False);
}
template <>
PyObject* PyValueParser::toPyObject(std::string str) {
    return Py_BuildValue("s#", str.c_str(), str.length());
}

void PyValueParser::setProperty(Property* p, PyObject* args) {
    std::string className = p->getClassIdentifier();

    if (className == "org.inviwo.BoolProperty")
         static_cast<BoolProperty*>(p)->set(parse<bool>(args));
    else if (className == "org.inviwo.FloatProperty")
        static_cast<FloatProperty*>(p)->set(parse<float>(args));
    else if (className == "org.inviwo.DoubleProperty")
        static_cast<DoubleProperty*>(p)->set(parse<float>(args));
    else if (className == "org.inviwo.IntProperty")
        static_cast<IntProperty*>(p)->set(parse<int>(args));
    else if (className == "org.inviwo.StringProperty")
        static_cast<StringProperty*>(p)->set(parse<std::string>(args));
    else if (className == "org.inviwo.FileProperty")
        static_cast<FileProperty*>(p)->set(parse<std::string>(args));
    else if (className == "org.inviwo.DirectoryProperty")
        static_cast<DirectoryProperty*>(p)->set(parse<std::string>(args));
    else if (className == "org.inviwo.IntVec2Property")
        static_cast<IntVec2Property*>(p)->set(parse<ivec2>(args));
    else if (className == "org.inviwo.IntVec3Property")
        static_cast<IntVec3Property*>(p)->set(parse<ivec3>(args));
    else if (className == "org.inviwo.IntVec4Property")
        static_cast<IntVec4Property*>(p)->set(parse<ivec4>(args));
    else if (className == "org.inviwo.FloatVec2Property")
        static_cast<FloatVec2Property*>(p)->set(parse<vec2>(args));
    else if (className == "org.inviwo.FloatVec3Property")
        static_cast<FloatVec3Property*>(p)->set(parse<vec3>(args));
    else if (className == "org.inviwo.FloatVec4Property")
        static_cast<FloatVec4Property*>(p)->set(parse<vec4>(args));
    else if (className == "org.inviwo.FloatMat2Property")
        static_cast<FloatVec2Property*>(p)->set(parse<vec2>(args));
    else if (className == "org.inviwo.FloatMat3Property")
        static_cast<FloatVec3Property*>(p)->set(parse<vec3>(args));
    else if (className == "org.inviwo.FloatMat4Property")
        static_cast<FloatVec4Property*>(p)->set(parse<vec4>(args));
    else if (className == "org.inviwo.FloatMinMaxProperty")
        static_cast<FloatMinMaxProperty*>(p)->set(parse<vec2>(args));
    else if (className == "org.inviwo.DoubleVec2Property")
        static_cast<DoubleVec2Property*>(p)->set(parse<dvec2>(args));
    else if (className == "org.inviwo.DoubleVec3Property")
        static_cast<DoubleVec3Property*>(p)->set(parse<dvec3>(args));
    else if (className == "org.inviwo.DoubleVec4Property")
        static_cast<DoubleVec4Property*>(p)->set(parse<dvec4>(args));
    else if (className == "org.inviwo.DoubleMat2Property")
        static_cast<DoubleVec2Property*>(p)->set(parse<dvec2>(args));
    else if (className == "org.inviwo.DoubleMat3Property")
        static_cast<DoubleVec3Property*>(p)->set(parse<dvec3>(args));
    else if (className == "org.inviwo.DoubleMat4Property")
        static_cast<DoubleVec4Property*>(p)->set(parse<dvec4>(args));
    else if (className == "org.inviwo.DoubleMinMaxProperty")
        static_cast<DoubleMinMaxProperty*>(p)->set(parse<dvec2>(args));
    else if (className == "org.inviwo.OptionPropertyInt")
        static_cast<OptionPropertyInt*>(p)->set(parse<int>(args));
    else if (className == "org.inviwo.OptionPropertyFloat")
        static_cast<OptionPropertyFloat*>(p)->set(parse<float>(args));
    else if (className == "org.inviwo.OptionPropertyString")
        static_cast<OptionPropertyString*>(p)->set(parse<std::string>(args));
    else if (className == "org.inviwo.OptionPropertyDouble")
        static_cast<OptionPropertyDouble*>(p)->set(parse<double>(args));
    else if (className == "org.inviwo.IntMinMaxProperty")
        static_cast<IntMinMaxProperty*>(p)->set(parse<ivec2>(args));
    else if (className == "org.inviwo.CameraProperty") {
        vec3 from, to, up;

        // float fovy,nearP,farP;
        if (!PyArg_ParseTuple(args, "(fff)(fff)(fff)", &from.x, &from.y, &from.z, &to.x, &to.y,
                              &to.z, &up.x, &up.y, &up.z
                                                    //,&fovy,&nearP,&farP
                              )) {
            std::string msg = std::string(
                                  "Failed to parse values for camera, needs to be on the format: "
                                  "((posX,posY,posZ),(focusX,focusY,focusZ),(upX,upY,upZ)) : ") +
                              p->getIdentifier();
            PyErr_SetString(PyExc_TypeError, msg.c_str());
            return;
        }

        CameraProperty* cam = static_cast<CameraProperty*>(p);
        cam->setLook(from, to, up);
    } else {
        LogWarnCustom("PyValueParser", "Unknown Property type : " << className);
        std::string msg =
            std::string("setPropertyValue() no available conversion for proerty of type: ") +
            className;
        PyErr_SetString(PyExc_TypeError, msg.c_str());
    }
}

#define CAST_DO_STUFF(PropType, prop)                     \
    {                                                     \
        PropType* casted = dynamic_cast<PropType*>(prop); \
        if (casted) {                                     \
            return toPyObject(casted->get());             \
        }                                                 \
    }

PyObject* PyValueParser::getProperty(Property* p) {
    CAST_DO_STUFF(BoolProperty, p);
    CAST_DO_STUFF(FloatProperty, p);
    CAST_DO_STUFF(IntProperty, p);
    CAST_DO_STUFF(StringProperty, p);
    CAST_DO_STUFF(FileProperty, p);
    CAST_DO_STUFF(DirectoryProperty, p);
    CAST_DO_STUFF(IntVec2Property, p);
    CAST_DO_STUFF(IntVec3Property, p);
    CAST_DO_STUFF(IntVec4Property, p);
    CAST_DO_STUFF(FloatVec2Property, p);
    CAST_DO_STUFF(FloatVec3Property, p);
    CAST_DO_STUFF(FloatVec4Property, p);
    CAST_DO_STUFF(DoubleVec2Property, p);
    CAST_DO_STUFF(DoubleVec3Property, p);
    CAST_DO_STUFF(DoubleVec4Property, p);
    CAST_DO_STUFF(FloatMat2Property, p);
    CAST_DO_STUFF(FloatMat3Property, p);
    CAST_DO_STUFF(FloatMat4Property, p);
    CAST_DO_STUFF(FloatMinMaxProperty, p);
    CAST_DO_STUFF(IntMinMaxProperty, p);
    CAST_DO_STUFF(DoubleMinMaxProperty, p);
    CAST_DO_STUFF(OptionPropertyInt, p);
    CAST_DO_STUFF(OptionPropertyFloat, p);
    CAST_DO_STUFF(OptionPropertyString, p);
    CAST_DO_STUFF(OptionPropertyDouble, p);

    CameraProperty* casted = dynamic_cast<CameraProperty*>(p);
    if (casted) {
        vec3 from = casted->getLookFrom();
        vec3 to = casted->getLookTo();
        vec3 up = casted->getLookUp();

        return Py_BuildValue("(fff)(fff)(fff)", from.x, from.y, from.z, to.x, to.y, to.z, up.x,
                             up.y, up.z);
    }
    std::string msg = std::string("Could create a python value of property  ") +
                      p->getIdentifier() + " which is of type " + p->getClassIdentifier();
    PyErr_SetString(PyExc_TypeError, msg.c_str());
    return nullptr;
}

}  // namespace
