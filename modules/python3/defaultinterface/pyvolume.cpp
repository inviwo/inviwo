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

#include "pyvolume.h"

#include <modules/python3/pythoninterface/pyvalueparser.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/processors/processor.h>
#include <modules/opengl/canvasprocessorgl.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/util/filesystem.h>

namespace inviwo {


PyObject* py_saveTransferFunction(PyObject* /*self*/, PyObject* args) {
    static PySaveTransferFunction p;

    if (!p.testParams(args)) return nullptr;

    std::string path = std::string(PyValueParser::parse<std::string>(PyTuple_GetItem(args, 0)));
    Property* theProperty = InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));

    if (!theProperty) {
        std::string msg = std::string("setPropertyValue() no property with path: ") + path;
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }


    std::string filename      = PyValueParser::parse<std::string>(PyTuple_GetItem(args, 1));
    
    TransferFunctionProperty* tf = dynamic_cast<TransferFunctionProperty*>(theProperty);

    if (!tf) {
        std::string msg = std::string("saveTransferFunction() no transfer function property at path: ") + ", (" + path
                          +" is of type "+ theProperty->getClassIdentifier() +  ")";
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }

    //*
    IvwSerializer serializer(filename);
    tf->serialize(serializer);
    serializer.writeFile();
    //*/
    Py_RETURN_NONE;
}


PyObject* py_loadTransferFunction(PyObject* /*self*/, PyObject* args) {
    static PyLoadTransferFunction p;

    if (!p.testParams(args)) return nullptr;

    std::string path = std::string(PyValueParser::parse<std::string>(PyTuple_GetItem(args, 0)));
    Property* theProperty = InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));

    if (!theProperty) {
        std::string msg = std::string("setPropertyValue() no property with path: ") + path;
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }
    
    std::string filename = PyValueParser::parse<std::string>(PyTuple_GetItem(args, 1));
    
    TransferFunctionProperty* tf = dynamic_cast<TransferFunctionProperty*>(theProperty);

    if (!tf) {
        std::string msg = std::string("loadTransferFunction() no transfer function property with id: ") + path + ", ("+path
                          +" is of type "+ theProperty->getClassIdentifier() +  ")";
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }

    if (!filesystem::fileExists(filename)) {
        if (filesystem::fileExists(InviwoApplication::getPtr()->getPath(InviwoApplication::PATH_TRANSFERFUNCTIONS, "/" + filename))) {
            filename = InviwoApplication::getPtr()->getPath(InviwoApplication::PATH_TRANSFERFUNCTIONS, "/" + filename);
        } else {
            std::string msg = "loadTransferFunction() file not found (" + filename + ")";
            PyErr_SetString(PyExc_TypeError, msg.c_str());
            return nullptr;
        }
    }

    //*
    IvwDeserializer deserializer(filename);
    tf->deserialize(deserializer);
    //*/
    Py_RETURN_NONE;
}


PyObject* py_clearTransferfunction(PyObject* /*self*/, PyObject* args) {
    static PyClearTransferfunction p;

    if (!p.testParams(args)) return nullptr;
    std::string path = std::string(PyValueParser::parse<std::string>(PyTuple_GetItem(args, 0)));

    Property* theProperty = InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));

    if (!theProperty) {
        std::string msg = std::string("setPropertyValue() no property with path: ") + path;
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }
    TransferFunctionProperty* tf = dynamic_cast<TransferFunctionProperty*>(theProperty);

    if (!tf) {
        std::string msg = std::string("clearTransferfunction() no transfer function property with id: ") + path + ", ("+path
                          +" is of type "+ theProperty->getClassIdentifier() +  ")";
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }

    tf->get().clearPoints();
    Py_RETURN_NONE;
}


PyObject* py_addPointTransferFunction(PyObject* /*self*/, PyObject* args) {
    static PyAddTransferFunction p;

    if (!p.testParams(args)) return nullptr;

    std::string path = std::string(PyValueParser::parse<std::string>(PyTuple_GetItem(args, 0)));
    Property* theProperty = InviwoApplication::getPtr()->getProcessorNetwork()->getProperty(splitString(path, '.'));

    if (!theProperty) {
        std::string msg = std::string("setPropertyValue() no property with path: ") + path;
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }

    vec2 pos = PyValueParser::parse<vec2>(PyTuple_GetItem(args, 1));
    vec3 color = PyValueParser::parse<vec3>(PyTuple_GetItem(args, 2));
    
    TransferFunctionProperty* tf = dynamic_cast<TransferFunctionProperty*>(theProperty);

    if (!tf) {
        std::string msg = std::string("addPointToTransferFunction() no transfer function property with id: ") + path + ", ("+path
                          +" is of type "+ theProperty->getClassIdentifier() +  ")";
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }

    tf->get().addPoint(pos,vec4(color,pos.y));
    tf->setPropertyModified(true);
    Py_RETURN_NONE;
}


PySaveTransferFunction::PySaveTransferFunction()
    : path_("path")
    , filename_("filename")
{
    addParam(&path_);
    addParam(&filename_);
}



PyLoadTransferFunction::PyLoadTransferFunction()
    : path_("path")
    , filename_("filename")
{
    addParam(&path_);
    addParam(&filename_);
}


PyClearTransferfunction::PyClearTransferfunction()
    : path_("path")
{
    addParam(&path_);
}




PyAddTransferFunction::PyAddTransferFunction()
    : path_("path")
    , pos_("position")
    , color_("color")
{
    addParam(&path_);
    addParam(&pos_);
    addParam(&color_);
}

}



