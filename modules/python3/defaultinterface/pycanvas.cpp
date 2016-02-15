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

#include "pycanvas.h"

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetwork.h>

#include <modules/opengl/canvasprocessorgl.h>
#include <modules/python3/pythoninterface/pyvalueparser.h>
#include <modules/python3/pythoninterface/pythonparameterparser.h>

namespace inviwo {

PyObject* py_canvascount(PyObject* /*self*/, PyObject* /*args*/) {
    if (InviwoApplication::getPtr() && InviwoApplication::getPtr()->getProcessorNetwork()) {
        std::vector<CanvasProcessor*> canvases = InviwoApplication::getPtr()
                                                     ->getProcessorNetwork()
                                                     ->getProcessorsByType<CanvasProcessor>();
        return PyValueParser::toPyObject(canvases.size());
    }

    Py_RETURN_NONE;
}

PyObject* py_resizecanvas(PyObject* /*self*/, PyObject* args) {
    static PythonParameterParser tester;
    PyObject* arg0;
    int w;
    int h;
    if (tester.parse(args,arg0, w, h) == -1) {
        return nullptr;
    }

   

    CanvasProcessor* canvas = nullptr;
    bool argIsString = PyValueParser::is<std::string>(arg0);

    if (argIsString) {
        std::string id = PyValueParser::parse<std::string>(arg0);
        Processor* processor =
            InviwoApplication::getPtr()->getProcessorNetwork()->getProcessorByIdentifier(id);

        if (!processor) {
            std::string msg =
                std::string("resizeCanvas(canvas,width,height) no processor with name: ") + id;
            PyErr_SetString(PyExc_TypeError, msg.c_str());
            return nullptr;
        }

        canvas = dynamic_cast<CanvasProcessor*>(processor);

        if (!canvas) {
            std::string msg =
                std::string("resizeCanvas(canvas,width,height) processor with name: ") + id +
                " is not a canvas processor, it is a" + processor->getClassIdentifier();
            PyErr_SetString(PyExc_TypeError, msg.c_str());
            return nullptr;
        }
    } else {
        int id = PyValueParser::parse<int>(arg0);
        std::vector<CanvasProcessor*> canvases = InviwoApplication::getPtr()
                                                     ->getProcessorNetwork()
                                                     ->getProcessorsByType<CanvasProcessor>();

        if (canvases.size() == 0) {
            std::string msg = std::string(
                "resizeCanvas(canvas,width,height) no canvases found in current network");
            PyErr_SetString(PyExc_TypeError, msg.c_str());
            return nullptr;
        }

        if (static_cast<int>(canvases.size()) <= id) {
            std::string msg =
                std::string(
                    "resizeCanvas(canvas,width,height) index out of bounds, index given: ") +
                toString(id) + ", max index possible: " + toString(canvases.size() - 1);
            PyErr_SetString(PyExc_TypeError, msg.c_str());
            return nullptr;
        }

        canvas = canvases[id];
    }


    if (w <= 0 || h <= 0) {
        std::string msg = std::string(
            "resizeCanvas(canvas,width,height) width and height must have positive non-zero "
            "values");
        PyErr_SetString(PyExc_TypeError, msg.c_str());
        return nullptr;
    }

    canvas->setCanvasSize(ivec2(w, h));
    //canvas->invalidate(InvalidationLevel::InvalidOutput);

    Py_RETURN_NONE;
}

}