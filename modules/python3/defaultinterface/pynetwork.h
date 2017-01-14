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

#ifndef IVW_PYNETWORK_H
#define IVW_PYNETWORK_H

#include <modules/python3/python3moduledefine.h>

namespace inviwo {

PyObject* py_getFactoryProcessors(PyObject* /*self*/, PyObject* /*args*/);

PyObject* py_getProcessors(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_addProcessor(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_removeProcessor(PyObject* /*self*/, PyObject* /*args*/);

PyObject* py_getConnections(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_addConnection(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_removeConnection(PyObject* /*self*/, PyObject* /*args*/);

PyObject* py_getLinks(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_addLink(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_removeLink(PyObject* /*self*/, PyObject* /*args*/);

PyObject* py_clearNetwork(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_loadNetwork(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_saveNetwork(PyObject* /*self*/, PyObject* /*args*/);

} // namespace

#endif // IVW_PYNETWORK_H

