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
#include <modules/python3/pythoninterface/pyvalueparser.h>


namespace inviwo {


PyObject* py_listProperties(PyObject* /*self*/, PyObject* args) {
    static PyListPropertiesMethod p;

    if (!p.testParams(args))
        return 0;

    if (PyTuple_Size(args) != 1) {
        std::ostringstream errStr;
        errStr << "listProperties() takes exactly 1 argument: processor name";
        errStr << " (" << PyTuple_Size(args) << " given)";
        PyErr_SetString(PyExc_TypeError, errStr.str().c_str());
        return 0;
    }

    // check parameter if is string
    if (!PyValueParser::is<std::string>(PyTuple_GetItem(args, 0))) {
        PyErr_SetString(PyExc_TypeError, "listProperties() argument must be a string");
        return 0;
    }

    std::string processorName = PyValueParser::parse<std::string>(PyTuple_GetItem(args, 0));
    Processor* processor = InviwoApplication::getPtr()->getProcessorNetwork()->getProcessorByIdentifier(processorName);

    if (!processor) {
        std::ostringstream errStr;
        errStr << "listProperties(): no processor with name " << processorName << " could be found";
        PyErr_SetString(PyExc_TypeError, errStr.str().c_str());
    } else {
        std::vector<Property*> props = processor->getProperties();

        for (std::vector<Property*>::const_iterator p = props.begin(); p != props.end(); ++p) {
            std::string name = (*p)->getIdentifier();
            std::string type  = (*p)->getClassIdentifier();
            PyRun_SimpleString(("print(\""+ name + " : "+ type + "\")").c_str());
        }
    }

    Py_RETURN_NONE;
}



PyObject* py_listProcesoors(PyObject* /*self*/, PyObject* /*args*/) {
    if (InviwoApplication::getPtr() && InviwoApplication::getPtr()->getProcessorNetwork()) {
        std::vector<Processor*> processors  = InviwoApplication::getPtr()->getProcessorNetwork()->getProcessors();

        for (std::vector<Processor*>::const_iterator processor = processors.begin(); processor!=processors.end(); ++processor) {
            std::string name = (*processor)->getIdentifier();
            std::string type = (*processor)->getClassIdentifier();
            PyRun_SimpleString(("print(\""+ name + " : "+ type + "\")").c_str());
        }
    }

    Py_RETURN_NONE;
}

PyListPropertiesMethod::PyListPropertiesMethod()
    : processor_("processor")
{
    addParam(&processor_);
}

}