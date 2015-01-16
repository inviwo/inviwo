/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#ifndef IVW_PYMODULE_H
#define IVW_PYMODULE_H

#include <modules/python3/python3moduledefine.h>

#include <modules/python3/pythoninterface/pymethod.h>

#include <modules/python3/defaultinterface/pyinfo.h>

#include <vector>
#include <string>
#include <map>


namespace inviwo {

class IVW_MODULE_PYTHON3_API PyModule {
public:
    PyModule(std::string moduleName);
    ~PyModule();
    void addMethod(PyMethod* method);

    const char* getModuleName();

    PyMethodDef* getPyMethodDefs();

    void printInfo();

    std::vector<PyMethod*> getPyMethods();

    void setPyObject(PyObject* obj);

    static PyModule* getModuleByPyObject(PyObject* obj);

    static std::map<PyObject*, PyModule*>::iterator begin(){ return instances_.begin(); }
    static std::map<PyObject*, PyModule*>::iterator end(){ return instances_.end(); }


private:
    std::string moduleName_;
    std::vector<PyMethod*> methods_;
    PyMethodDef* embMethods_;
    static std::map<PyObject*,PyModule*> instances_;


};


}//namespace

#endif
