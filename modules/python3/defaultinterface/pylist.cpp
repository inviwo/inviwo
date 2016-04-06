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

#include "pylist.h"

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <modules/python3/pythoninterface/pyvalueparser.h>
#include <modules/python3/pythoninterface/pythonparameterparser.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/network/processornetwork.h>

namespace inviwo {

PyObject* py_listProperties(PyObject* /*self*/, PyObject* args) {
    static PythonParameterParser tester;
    std::string processorName;
    if (tester.parse<std::string>(args, processorName) == -1) {
        return nullptr;
    }

    auto network = InviwoApplication::getPtr()->getProcessorNetwork();

    if (auto processor = network->getProcessorByIdentifier(processorName)) {
        auto props = processor->getPropertiesRecursive();

        PyObject* lst = PyList_New(props.size());
        if (!lst) Py_RETURN_NONE;

        int i = 0;
        for (auto prop : props) {
            std::string name = joinString(prop->getPath(), ".");
            std::string type = prop->getClassIdentifier();
            PyObject* pair =
                Py_BuildValue("(s#s#)", name.c_str(), name.size(), type.c_str(), type.size());
            if (!pair) {
                Py_DECREF(lst);
                Py_RETURN_NONE;
            }

            PyList_SET_ITEM(lst, i, pair);
            ++i;
        }
        return lst;
    } else {
        std::ostringstream errStr;
        errStr << "listProperties(): no processor with name " << processorName << " could be found";
        PyErr_SetString(PyExc_TypeError, errStr.str().c_str());
    }

    Py_RETURN_NONE;
}

PyObject* py_listProcessors(PyObject* /*self*/, PyObject* /*args*/) {
    if (InviwoApplication::getPtr() && InviwoApplication::getPtr()->getProcessorNetwork()) {
        auto processors = InviwoApplication::getPtr()->getProcessorNetwork()->getProcessors();

        PyObject* lst = PyList_New(processors.size());
        if (!lst) Py_RETURN_NONE;

        int i = 0;
        for (auto processor : processors) {
            std::string name = processor->getIdentifier();
            std::string type = processor->getClassIdentifier();

            PyObject* pair =
                Py_BuildValue("(s#s#)", name.c_str(), name.size(), type.c_str(), type.size());
            if (!pair) {
                Py_DECREF(lst);
                Py_RETURN_NONE;
            }

            PyList_SET_ITEM(lst, i, pair);
            ++i;
        }
        return lst;
    }

    Py_RETURN_NONE;
}

PyObject* py_listCanvases(PyObject* /*self*/, PyObject* /*noargs*/) {
    if (InviwoApplication::getPtr() && InviwoApplication::getPtr()->getProcessorNetwork()) {
        auto processors = InviwoApplication::getPtr()->getProcessorNetwork()->getProcessors();

        util::erase_remove_if(processors, [](const Processor* p) {
            return dynamic_cast<const CanvasProcessor*>(p) == nullptr;
        });

        PyObject* lst = PyList_New(processors.size());
        if (!lst) Py_RETURN_NONE;

        int i = 0;
        for (auto processor : processors) {
            std::string name = processor->getIdentifier();
            std::string type = processor->getClassIdentifier();

            PyObject* pair =
                Py_BuildValue("(s#s#)", name.c_str(), name.size(), type.c_str(), type.size());
            if (!pair) {
                Py_DECREF(lst);
                Py_RETURN_NONE;
            }

            PyList_SET_ITEM(lst, i, pair);
            ++i;
        }
        return lst;
    }

    Py_RETURN_NONE;
}
}