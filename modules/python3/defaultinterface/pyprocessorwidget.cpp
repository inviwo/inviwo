/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#include <modules/python3/defaultinterface/pyprocessorwidget.h>
#include <modules/python3/pythoninterface/pythonparameterparser.h>
#include <modules/python3/pythoninterface/pyvalueparser.h>
#include <modules/python3/defaultinterface/utilities.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

PyObject* py_hasProcessorWidget(PyObject* self, PyObject* args) {
    return utilpy::getProcessorProperty(args, [](Processor* p) { return p->hasProcessorWidget(); });
}

PyObject* py_setProcessorWidgetVisible(PyObject* self, PyObject* args) {
    return utilpy::setProcessorProperty<bool>(args, [](Processor* p, const bool& visible) {
        if (auto w = p->getProcessorWidget()) {
            w->setVisible(visible);
        }
        Py_RETURN_NONE;
    });
}

PyObject* py_isProcessorWidgetVisible(PyObject* self, PyObject* args) {
    return utilpy::getProcessorProperty(args, [](Processor* p) {
        if (auto w = p->getProcessorWidget()) {
            return w->isVisible();
        }
        return false;
    });
}

PyObject* py_setProcessorWidgetPosition(PyObject* self, PyObject* args) {
    return utilpy::setProcessorProperty<ivec2>(args, [](Processor* p, const ivec2& pos) {
        if (auto w = p->getProcessorWidget()) {
            w->setPosition(pos);
        }
        Py_RETURN_NONE;
    });
}

PyObject* py_getProcessorWidgetPosition(PyObject* self, PyObject* args) {
    return utilpy::getProcessorProperty(args, [](Processor* p) {
        if (auto w = p->getProcessorWidget()) {
            return w->getPosition();
        }
        return ivec2(0, 0);
    });
}

PyObject* py_setProcessorWidgetDimensions(PyObject* self, PyObject* args) {
    return utilpy::setProcessorProperty<ivec2>(args, [](Processor* p, const ivec2& dim) {
        if (auto w = p->getProcessorWidget()) {
            w->setDimensions(dim);
        }
        Py_RETURN_NONE;
    });
}

PyObject* py_getProcessorWidgetDimensions(PyObject* self, PyObject* args) {
    return utilpy::getProcessorProperty(args, [](Processor* p) {
        if (auto w = p->getProcessorWidget()) {
            return w->getDimensions();
        }
        return ivec2(0, 0);
    });
}

}  // namespace
