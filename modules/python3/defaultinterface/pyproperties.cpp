/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include "pyproperties.h"

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/network/processornetwork.h>

#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>

#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <modules/python3/pythoninterface/pyvalueparser.h>
#include <modules/python3/pythoninterface/pythonparameterparser.h>

namespace inviwo {


    static std::pair<Property*,PyObject *> getProperty(PyObject* args, std::string methodName) {
        static PythonParameterParser tester;

         
        std::string path;
        PyObject* parameter;
        if (tester.parse<std::string, PyObject*>(args, path, parameter) == -1) {
            return std::make_pair<Property*, PyObject *>(nullptr, nullptr);
        }
        Property* theProperty =
            InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));

        if (!theProperty) {
            std::string msg = std::string(methodName + "() no property with path: ") + path;
            PyErr_SetString(PyExc_TypeError, msg.c_str());
            return std::make_pair<Property*, PyObject *>(nullptr, nullptr);
        }
        return std::make_pair(theProperty, parameter);
    }

PyObject* py_setPropertyValue(PyObject* self, PyObject* args) {
    auto propertyAndParameter = getProperty(args, "setPropertyValue");
    auto theProperty = propertyAndParameter.first;
    auto parameter = propertyAndParameter.second;
    if (!theProperty) return nullptr;

    if (!PyValueParser::setProperty(theProperty, parameter)) {
        return nullptr;
    }
    Py_RETURN_NONE;
}

PyObject* py_setPropertyMaxValue(PyObject* /*self*/, PyObject* args) {
    auto propertyAndParameter = getProperty(args, "setPropertyMaxValue");
    auto theProperty = propertyAndParameter.first;
    auto parameter = propertyAndParameter.second;
    if (!theProperty) return nullptr;

    OrdinalProperty<float>* ordinalFloat = dynamic_cast<OrdinalProperty<float>*>(theProperty);
    OrdinalProperty<double>* ordinalDouble = dynamic_cast<OrdinalProperty<double>*>(theProperty);
    OrdinalProperty<int>* ordinalInt = dynamic_cast<OrdinalProperty<int>*>(theProperty);
    OrdinalProperty<ivec2>* ordinalIvec2 = dynamic_cast<OrdinalProperty<ivec2>*>(theProperty);
    OrdinalProperty<ivec3>* ordinalIvec3 = dynamic_cast<OrdinalProperty<ivec3>*>(theProperty);
    OrdinalProperty<ivec4>* ordinalIvec4 = dynamic_cast<OrdinalProperty<ivec4>*>(theProperty);
    OrdinalProperty<mat2>* ordinalMat2 = dynamic_cast<OrdinalProperty<mat2>*>(theProperty);
    OrdinalProperty<mat3>* ordinalMat3 = dynamic_cast<OrdinalProperty<mat3>*>(theProperty);
    OrdinalProperty<mat4>* ordinalMat4 = dynamic_cast<OrdinalProperty<mat4>*>(theProperty);
    OrdinalProperty<vec2>* ordinalVec2 = dynamic_cast<OrdinalProperty<vec2>*>(theProperty);
    OrdinalProperty<vec3>* ordinalVec3 = dynamic_cast<OrdinalProperty<vec3>*>(theProperty);
    OrdinalProperty<vec4>* ordinalVec4 = dynamic_cast<OrdinalProperty<vec4>*>(theProperty);

    if (ordinalFloat) {
        ordinalFloat->setMaxValue(PyValueParser::parse<float>(parameter));
    } else if (ordinalDouble) {
        ordinalDouble->setMaxValue(PyValueParser::parse<double>(parameter));
    } else if (ordinalInt) {
        ordinalInt->setMaxValue(PyValueParser::parse<int>(parameter));
    } else if (ordinalIvec2) {
        ordinalIvec2->setMaxValue(PyValueParser::parse<ivec2>(parameter));
    } else if (ordinalIvec3) {
        ordinalIvec3->setMaxValue(PyValueParser::parse<ivec3>(parameter));
    } else if (ordinalIvec4) {
        ordinalIvec4->setMaxValue(PyValueParser::parse<ivec4>(parameter));
    } else if (ordinalMat2) {
        ordinalMat2->setMaxValue(PyValueParser::parse<mat2>(parameter));
    } else if (ordinalMat3) {
        ordinalMat3->setMaxValue(PyValueParser::parse<mat3>(parameter));
    } else if (ordinalMat4) {
        ordinalMat4->setMaxValue(PyValueParser::parse<mat4>(parameter));
    } else if (ordinalVec2) {
        ordinalVec2->setMaxValue(PyValueParser::parse<vec2>(parameter));
    } else if (ordinalVec3) {
        ordinalVec3->setMaxValue(PyValueParser::parse<vec3>(parameter));
    } else if (ordinalVec4) {
        ordinalVec4->setMaxValue(PyValueParser::parse<vec4>(parameter));
    } else {
        LogErrorCustom("inviwo_setPropertyMaxValue",
                       "Unknown parameter type: " << theProperty->getClassIdentifier());
    }

    Py_RETURN_NONE;
}

PyObject* py_setPropertyMinValue(PyObject* /*self*/, PyObject* args) {
    auto propertyAndParameter = getProperty(args, "setPropertyMinValue");
    auto theProperty = propertyAndParameter.first;
    auto parameter = propertyAndParameter.second;
    if (!theProperty) return nullptr;

    OrdinalProperty<float>* ordinalFloat = dynamic_cast<OrdinalProperty<float>*>(theProperty);
    OrdinalProperty<int>* ordinalInt = dynamic_cast<OrdinalProperty<int>*>(theProperty);
    OrdinalProperty<ivec2>* ordinalIvec2 = dynamic_cast<OrdinalProperty<ivec2>*>(theProperty);
    OrdinalProperty<ivec3>* ordinalIvec3 = dynamic_cast<OrdinalProperty<ivec3>*>(theProperty);
    OrdinalProperty<ivec4>* ordinalIvec4 = dynamic_cast<OrdinalProperty<ivec4>*>(theProperty);
    OrdinalProperty<mat2>* ordinalMat2 = dynamic_cast<OrdinalProperty<mat2>*>(theProperty);
    OrdinalProperty<mat3>* ordinalMat3 = dynamic_cast<OrdinalProperty<mat3>*>(theProperty);
    OrdinalProperty<mat4>* ordinalMat4 = dynamic_cast<OrdinalProperty<mat4>*>(theProperty);
    OrdinalProperty<vec2>* ordinalVec2 = dynamic_cast<OrdinalProperty<vec2>*>(theProperty);
    OrdinalProperty<vec3>* ordinalVec3 = dynamic_cast<OrdinalProperty<vec3>*>(theProperty);
    OrdinalProperty<vec4>* ordinalVec4 = dynamic_cast<OrdinalProperty<vec4>*>(theProperty);

    if (ordinalFloat) {
        ordinalFloat->setMinValue(PyValueParser::parse<float>(parameter));
    } else if (ordinalInt) {
        ordinalInt->setMinValue(PyValueParser::parse<int>(parameter));
    } else if (ordinalIvec2) {
        ordinalIvec2->setMinValue(PyValueParser::parse<ivec2>(parameter));
    } else if (ordinalIvec3) {
        ordinalIvec3->setMinValue(PyValueParser::parse<ivec3>(parameter));
    } else if (ordinalIvec4) {
        ordinalIvec4->setMinValue(PyValueParser::parse<ivec4>(parameter));
    } else if (ordinalMat2) {
        ordinalMat2->setMinValue(PyValueParser::parse<mat2>(parameter));
    } else if (ordinalMat3) {
        ordinalMat3->setMinValue(PyValueParser::parse<mat3>(parameter));
    } else if (ordinalMat4) {
        ordinalMat4->setMinValue(PyValueParser::parse<mat4>(parameter));
    } else if (ordinalVec2) {
        ordinalVec2->setMinValue(PyValueParser::parse<vec2>(parameter));
    } else if (ordinalVec3) {
        ordinalVec3->setMinValue(PyValueParser::parse<vec3>(parameter));
    } else if (ordinalVec4) {
        ordinalVec4->setMinValue(PyValueParser::parse<vec4>(parameter));
    } else {
        LogErrorCustom("inviwo_setPropertyMinValue", "Unknown parameter type");
    }

    Py_RETURN_NONE;
}

PyObject* py_getPropertyValue(PyObject* /*self*/, PyObject* args) {
    static PythonParameterParser tester;

     
    std::string path;
    if (tester.parse<std::string>(args, path) == -1) {
        return nullptr;
    }
    Property* theProperty =
        InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));

    if (!theProperty) {
        std::string msg = std::string("setPropertyValue() no property with path: ") + path;
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }

    return PyValueParser::getProperty(theProperty);
}

#define CAST_N_GETMAX(PropType, prop)                                \
    {                                                                \
        if (auto casted = dynamic_cast<PropType*>(prop)) {           \
            return PyValueParser::toPyObject(casted->getMaxValue()); \
        }                                                            \
    }
#define CAST_N_GETMIN(PropType, prop)                                \
    {                                                                \
        if (auto casted = dynamic_cast<PropType*>(prop)) {           \
            return PyValueParser::toPyObject(casted->getMinValue()); \
        }                                                            \
    }

PyObject* py_getPropertyMaxValue(PyObject* /*self*/, PyObject* args) {
    static PythonParameterParser tester;

     
    std::string path;
    if (tester.parse<std::string>(args, path) == -1) {
        return nullptr;
    }
    Property* theProperty =
        InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));

    if (!theProperty) {
        std::string msg = std::string("setPropertyValue() no property with path: ") + path;
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }

    CAST_N_GETMAX(FloatProperty, theProperty);
    CAST_N_GETMAX(DoubleProperty, theProperty);
    CAST_N_GETMAX(IntProperty, theProperty);
    CAST_N_GETMAX(IntVec2Property, theProperty);
    CAST_N_GETMAX(IntVec3Property, theProperty);
    CAST_N_GETMAX(IntVec4Property, theProperty);
    CAST_N_GETMAX(FloatMat2Property, theProperty);
    CAST_N_GETMAX(FloatMat3Property, theProperty);
    CAST_N_GETMAX(FloatMat4Property, theProperty);
    CAST_N_GETMAX(FloatVec2Property, theProperty);
    CAST_N_GETMAX(FloatVec3Property, theProperty);
    CAST_N_GETMAX(FloatVec4Property, theProperty);
    LogErrorCustom("inviwo_getPropertyMaxValue",
                   "Unknown parameter type: " << theProperty->getClassIdentifier());
    Py_RETURN_NONE;
}

PyObject* py_getPropertyMinValue(PyObject* /*self*/, PyObject* args) {
    static PythonParameterParser tester;

     
    std::string path;
    if (tester.parse<std::string>(args, path) == -1) {
        return nullptr;
    }
    Property* theProperty =
        InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));

    if (!theProperty) {
        std::string msg = std::string("setPropertyValue() no property with path: ") + path;
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }

    CAST_N_GETMIN(FloatProperty, theProperty);
    CAST_N_GETMIN(DoubleProperty, theProperty);
    CAST_N_GETMIN(IntProperty, theProperty);
    CAST_N_GETMIN(IntVec2Property, theProperty);
    CAST_N_GETMIN(IntVec3Property, theProperty);
    CAST_N_GETMIN(IntVec4Property, theProperty);
    CAST_N_GETMIN(FloatMat2Property, theProperty);
    CAST_N_GETMIN(FloatMat3Property, theProperty);
    CAST_N_GETMIN(FloatMat4Property, theProperty);
    CAST_N_GETMIN(FloatVec2Property, theProperty);
    CAST_N_GETMIN(FloatVec3Property, theProperty);
    CAST_N_GETMIN(FloatVec4Property, theProperty);
    LogErrorCustom("inviwo_getPropertyMaxValue",
                   "Unknown parameter type: " << theProperty->getClassIdentifier());
    Py_RETURN_NONE;
}

PyObject* py_clickButton(PyObject* /*self*/, PyObject* args) {
    static PythonParameterParser tester;

     
    std::string path;
    if (tester.parse<std::string>(args, path) == -1) {
        return nullptr;
    }
    Property* theProperty =
        InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));

    if (!theProperty) {
        std::string msg = std::string("setPropertyValue() no property with path: ") + path;
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }

    ButtonProperty* button = dynamic_cast<ButtonProperty*>(theProperty);

    if (!button) {
        std::string msg = std::string("clickButton() found property is not a ButtonProperty: ") +
                          theProperty->getClassIdentifier();
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }

    button->pressButton();
    Py_RETURN_NONE;
}
}