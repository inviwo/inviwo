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

#ifndef IVW_PYLISTMEHTODSINVIWO_H
#define IVW_PYLISTMEHTODSINVIWO_H



#include <modules/python3/python3moduledefine.h>

#include <modules/python3/pythoninterface/pymethod.h>

namespace inviwo {
PyObject* py_listProperties(PyObject* /*self*/, PyObject* /*args*/);
PyObject* py_listProcesoors(PyObject* /*self*/, PyObject* /*args*/);


class IVW_MODULE_PYTHON3_API PyListPropertiesMethod : public PyMethod {
public:
    PyListPropertiesMethod();
    virtual ~PyListPropertiesMethod() {}
    virtual std::string getName()const {return "listProperties";}
    virtual std::string getDesc()const {return "List all properties for a processor";}
    virtual PyCFunction getFunc() {return py_listProperties;}
private:
    PyParamString processor_;

};
class IVW_MODULE_PYTHON3_API PyListProcessorsMethod : public PyMethod {
public:
    virtual std::string getName()const {return "listProcessors";}
    virtual std::string getDesc()const {return "Lists all processors in the current network";}
    virtual PyCFunction getFunc() {return py_listProcesoors;}

};

} //namespace


#endif // IVW_PYLISTMEHTODSINVIWO_H


