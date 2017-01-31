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


#include <modules/python3/pythonincluder.h>

#include <modules/python3/defaultinterface/pyprocessor.h>
#include <modules/python3/pythoninterface/pythonparameterparser.h>
#include <modules/python3/pythoninterface/pyvalueparser.h>
#include <modules/python3/defaultinterface/utilities.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

PyObject* py_setProcessorSelected(PyObject* self, PyObject* args) {
    return utilpy::setProcessorMetaDataProperty(args, &ProcessorMetaData::setSelected);
}

PyObject* py_isProcessorSelected(PyObject* self, PyObject* args) {
   return utilpy::getProcessorMetaDataProperty(args, &ProcessorMetaData::isSelected);
}

PyObject* py_setProcessorVisible(PyObject* self, PyObject* args) {
    return utilpy::setProcessorMetaDataProperty(args, &ProcessorMetaData::setVisible);
}
PyObject* py_isProcessorVisible(PyObject* self, PyObject* args) {
    return utilpy::getProcessorMetaDataProperty(args, &ProcessorMetaData::isVisible);
}

PyObject* py_setProcessorPosition(PyObject* self, PyObject* args) {
    return utilpy::setProcessorMetaDataProperty(args, &ProcessorMetaData::setPosition);
}
PyObject* py_getProcessorPosition(PyObject* self, PyObject* args) {
    return utilpy::getProcessorMetaDataProperty(args, &ProcessorMetaData::getPosition);
}

PyObject* py_getProcessorClassIdentifier(PyObject* self, PyObject* args) {
    return utilpy::getProcessorProperty(args, [](Processor* p) { return p->getClassIdentifier(); });
}
PyObject* py_getProcessorDisplayName(PyObject* self, PyObject* args) {
    return utilpy::getProcessorProperty(args, [](Processor* p) { return p->getDisplayName(); });
}
PyObject* py_getProcessorCategory(PyObject* self, PyObject* args) {
    return utilpy::getProcessorProperty(args, [](Processor* p) { return p->getCategory(); });
}
PyObject* py_getProcessorState(PyObject* self, PyObject* args) {
    return utilpy::getProcessorProperty(
        args, [](Processor* p) { return Processor::getCodeStateString(p->getCodeState()); });
}
PyObject* py_getProcessorTags(PyObject* self, PyObject* args) {
    return utilpy::getProcessorProperty(args, [](Processor* p) { return p->getTags().getString(); });
}

PyObject* py_getProcessorInports(PyObject* self, PyObject* args) {
    return utilpy::getProcessorPropertyList(args, [](Processor* p) {
        return util::transform(p->getInports(), [](Inport* port) {
            return std::make_pair(port->getIdentifier(), port->getClassIdentifier());
        });
    });
}
PyObject* py_getProcessorOutports(PyObject* self, PyObject* args) {
    return utilpy::getProcessorPropertyList(args, [](Processor* p) {
        return util::transform(p->getOutports(), [](Outport* port) {
            return std::make_pair(port->getIdentifier(), port->getClassIdentifier());
        });
    });
}
PyObject* py_getProcessorProperties(PyObject* self, PyObject* args) {
    return utilpy::getProcessorPropertyList(args, [](Processor* p) {
        return util::transform(p->getProperties(), [](Property* prop) {
            return std::make_pair(prop->getIdentifier(), prop->getClassIdentifier());
        });
    });
}

} // namespace







