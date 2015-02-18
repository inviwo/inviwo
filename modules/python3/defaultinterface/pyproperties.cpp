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

#include <inviwo/core/properties/baseoptionproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/baseoptionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>

#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <modules/python3/pythoninterface/pyvalueparser.h>

namespace inviwo {

PyObject* py_setPropertyValue(PyObject* self, PyObject* args) {
    static PySetPropertyValueMethod p;

    if (!p.testParams(args))
        return 0;
    std::string path = std::string(PyValueParser::parse<std::string>(PyTuple_GetItem(args, 0)));
    PyObject* parameter = PyTuple_GetItem(args, 1);
        
    Property* theProperty = InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));

    if (!theProperty) {
        std::string msg = std::string("setPropertyValue() no property with path: ") + path;
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return 0;
    }

    PyValueParser::setProperty(theProperty,parameter);
    Py_RETURN_NONE;
}





PyObject* py_setPropertyMaxValue(PyObject* /*self*/, PyObject* args) {
    static PySetPropertyMaxValueMethod p;

    if (!p.testParams(args))
        return 0;

    std::string path = std::string(PyValueParser::parse<std::string>(PyTuple_GetItem(args, 0)));
    PyObject* parameter = PyTuple_GetItem(args, 1);

    Property* theProperty = InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));

    if (!theProperty) {
        std::string msg = std::string("setPropertyValue() no property with path: ") + path;
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return 0;
    }

    OrdinalProperty<float>* ordinalFloat = dynamic_cast<OrdinalProperty<float>*>(theProperty);
    OrdinalProperty<int>*   ordinalInt   = dynamic_cast<OrdinalProperty<int>  *>(theProperty);
    OrdinalProperty<ivec2>* ordinalIvec2 = dynamic_cast<OrdinalProperty<ivec2>*>(theProperty);
    OrdinalProperty<ivec3>* ordinalIvec3 = dynamic_cast<OrdinalProperty<ivec3>*>(theProperty);
    OrdinalProperty<ivec4>* ordinalIvec4 = dynamic_cast<OrdinalProperty<ivec4>*>(theProperty);
    OrdinalProperty<mat2>*  ordinalMat2  = dynamic_cast<OrdinalProperty<mat2> *>(theProperty);
    OrdinalProperty<mat3>*  ordinalMat3  = dynamic_cast<OrdinalProperty<mat3> *>(theProperty);
    OrdinalProperty<mat4>*  ordinalMat4  = dynamic_cast<OrdinalProperty<mat4> *>(theProperty);
    OrdinalProperty<vec2>*  ordinalVec2  = dynamic_cast<OrdinalProperty<vec2> *>(theProperty);
    OrdinalProperty<vec3>*  ordinalVec3  = dynamic_cast<OrdinalProperty<vec3> *>(theProperty);
    OrdinalProperty<vec4>*  ordinalVec4  = dynamic_cast<OrdinalProperty<vec4> *>(theProperty);

    if (ordinalFloat) {
        ordinalFloat->setMaxValue(PyValueParser::parse<float>(parameter));
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
        LogErrorCustom("inviwo_setPropertyMaxValue","Unknown parameter type: " << theProperty->getClassIdentifier());
    }

    Py_RETURN_NONE;
}



PyObject* py_setPropertyMinValue(PyObject* /*self*/, PyObject* args) {
    static PySetPropertyMinValueMethod p;

    if (!p.testParams(args))
        return 0;

    std::string path = std::string(PyValueParser::parse<std::string>(PyTuple_GetItem(args, 0)));
    PyObject* parameter = PyTuple_GetItem(args, 1);

    Property* theProperty = InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));

    if (!theProperty) {
        std::string msg = std::string("setPropertyValue() no property with path: ") + path;
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return 0;
    }

    OrdinalProperty<float>* ordinalFloat = dynamic_cast<OrdinalProperty<float>*>(theProperty);
    OrdinalProperty<int>*   ordinalInt   = dynamic_cast<OrdinalProperty<int>  *>(theProperty);
    OrdinalProperty<ivec2>* ordinalIvec2 = dynamic_cast<OrdinalProperty<ivec2>*>(theProperty);
    OrdinalProperty<ivec3>* ordinalIvec3 = dynamic_cast<OrdinalProperty<ivec3>*>(theProperty);
    OrdinalProperty<ivec4>* ordinalIvec4 = dynamic_cast<OrdinalProperty<ivec4>*>(theProperty);
    OrdinalProperty<mat2>*  ordinalMat2  = dynamic_cast<OrdinalProperty<mat2> *>(theProperty);
    OrdinalProperty<mat3>*  ordinalMat3  = dynamic_cast<OrdinalProperty<mat3> *>(theProperty);
    OrdinalProperty<mat4>*  ordinalMat4  = dynamic_cast<OrdinalProperty<mat4> *>(theProperty);
    OrdinalProperty<vec2>*  ordinalVec2  = dynamic_cast<OrdinalProperty<vec2> *>(theProperty);
    OrdinalProperty<vec3>*  ordinalVec3  = dynamic_cast<OrdinalProperty<vec3> *>(theProperty);
    OrdinalProperty<vec4>*  ordinalVec4  = dynamic_cast<OrdinalProperty<vec4> *>(theProperty);

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
        LogErrorCustom("inviwo_setPropertyMinValue","Unknown parameter type");
    }

    Py_RETURN_NONE;
}




PyObject* py_getPropertyValue(PyObject* /*self*/, PyObject* args) {
    static PyGetPropertyValueMethod p;

    if (!p.testParams(args))
        return 0;

    std::string path = std::string(PyValueParser::parse<std::string>(PyTuple_GetItem(args, 0)));

    Property* theProperty = InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));

    if (!theProperty) {
        std::string msg = std::string("setPropertyValue() no property with path: ") + path;
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return 0;
    }

    return PyValueParser::getProperty(theProperty);
}

#define CAST_N_GETMAX(PropType,prop,parser) {PropType* casted = dynamic_cast<PropType*>(prop); if(casted) {return PyValueParser::toPyObject(casted->getMaxValue());}}
#define CAST_N_GETMIN(PropType,prop,parser) {PropType* casted = dynamic_cast<PropType*>(prop); if(casted) {return PyValueParser::toPyObject(casted->getMinValue());}}

PyObject* py_getPropertyMaxValue(PyObject* /*self*/, PyObject* args) {
    static PyGetPropertyMaxValueMethod p;

    if (!p.testParams(args))
        return 0;

    std::string path = std::string(PyValueParser::parse<std::string>(PyTuple_GetItem(args, 0)));

    Property* theProperty = InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));

    if (!theProperty) {
        std::string msg = std::string("setPropertyValue() no property with path: ") + path;
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return 0;
    }

    CAST_N_GETMAX(FloatProperty,theProperty,parser);
    CAST_N_GETMAX(IntProperty,theProperty,parser);
    CAST_N_GETMAX(IntVec2Property,theProperty,parser);
    CAST_N_GETMAX(IntVec3Property,theProperty,parser);
    CAST_N_GETMAX(IntVec4Property,theProperty,parser);
    CAST_N_GETMAX(FloatMat2Property,theProperty,parser);
    CAST_N_GETMAX(FloatMat3Property,theProperty,parser);
    CAST_N_GETMAX(FloatMat4Property,theProperty,parser);
    CAST_N_GETMAX(FloatVec2Property,theProperty,parser);
    CAST_N_GETMAX(FloatVec3Property,theProperty,parser);
    CAST_N_GETMAX(FloatVec4Property,theProperty,parser);
    LogErrorCustom("inviwo_getPropertyMaxValue","Unknown parameter type");
    Py_RETURN_NONE;
}



PyObject* py_getPropertyMinValue(PyObject* /*self*/, PyObject* args) {
    static PyGetPropertyMinValueMethod p;

    if (!p.testParams(args))
        return 0;

    std::string path = std::string(PyValueParser::parse<std::string>(PyTuple_GetItem(args, 0)));

    Property* theProperty = InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));

    if (!theProperty) {
        std::string msg = std::string("setPropertyValue() no property with path: ") + path;
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return 0;
    }

    CAST_N_GETMIN(FloatProperty,theProperty,parser);
    CAST_N_GETMIN(IntProperty,theProperty,parser);
    CAST_N_GETMIN(IntVec2Property,theProperty,parser);
    CAST_N_GETMIN(IntVec3Property,theProperty,parser);
    CAST_N_GETMIN(IntVec4Property,theProperty,parser);
    CAST_N_GETMIN(FloatMat2Property,theProperty,parser);
    CAST_N_GETMIN(FloatMat3Property,theProperty,parser);
    CAST_N_GETMIN(FloatMat4Property,theProperty,parser);
    CAST_N_GETMIN(FloatVec2Property,theProperty,parser);
    CAST_N_GETMIN(FloatVec3Property,theProperty,parser);
    CAST_N_GETMIN(FloatVec4Property,theProperty,parser);
    LogErrorCustom("inviwo_getPropertyMaxValue","Unknown parameter type");
    Py_RETURN_NONE;
}



PyObject* py_clickButton(PyObject* /*self*/, PyObject* args) {
    static PyClickButtonMethod p;

    if (!p.testParams(args))
        return 0;

    std::string path = std::string(PyValueParser::parse<std::string>(PyTuple_GetItem(args, 0)));
    
    Property* theProperty = InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));

    if (!theProperty) {
        std::string msg = std::string("setPropertyValue() no property with path: ") + path;
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return 0;
    }

    ButtonProperty* button = dynamic_cast<ButtonProperty*>(theProperty);

    if (!button) {
        std::string msg = std::string("clickButton() found property is not a ButtonProperty: ") + theProperty->getClassIdentifier();
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return 0;
    }

    button->propertyModified();
    Py_RETURN_NONE;
}


PySetPropertyValueMethod::PySetPropertyValueMethod()
    : path_("path")
    , value_("value")
{
    addParam(&path_);
    addParam(&value_);
}

PySetPropertyMaxValueMethod::PySetPropertyMaxValueMethod()
    : path_("path")
    , maxValue_("maxValue")
{
    addParam(&path_);
    addParam(&maxValue_);
}

PySetPropertyMinValueMethod::PySetPropertyMinValueMethod()
    : path_("path")
    , minValue_("minValue")
{
    addParam(&path_);
    addParam(&minValue_);
}

PyGetPropertyValueMethod::PyGetPropertyValueMethod()
    : path_("path")
{
    addParam(&path_);
}

PyGetPropertyMaxValueMethod::PyGetPropertyMaxValueMethod()
    : path_("path")
{
    addParam(&path_);
}

PyGetPropertyMinValueMethod::PyGetPropertyMinValueMethod()
    : path_("path")
{
    addParam(&path_);
}

PyClickButtonMethod::PyClickButtonMethod()
    : path_("path")
{
    addParam(&path_);
}




}