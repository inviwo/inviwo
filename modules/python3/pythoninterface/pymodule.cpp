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
#include <modules/python3/pythoninterface/pymodule.h>

#include <modules/python3/pyinviwo.h>
#include <inviwo/core/util/logcentral.h>
#include <modules/python3/pythoninterface/pyvalueparser.h>

namespace inviwo {

std::map<PyObject*,PyModule*> PyModule::instances_;

PyModule::PyModule(std::string moduleName) : obj_(nullptr) , moduleName_(moduleName), embMethods_(nullptr){
    addMethod(new PyInfoMethod());
}

PyModule::~PyModule() {
    while (!methods_.empty()) {delete methods_.back(); methods_.pop_back();}
    delete embMethods_;
}

PyMethodDef* PyModule::getPyMethodDefs(){
    size_t N = methods_.size();
    if(embMethods_) delete embMethods_;
    embMethods_ = new PyMethodDef[N + 1];
    
    for (size_t i = 0; i < N; i++){
        embMethods_[i].ml_name = methods_[i]->getName2();
        embMethods_[i].ml_meth = methods_[i]->getFunc();
        embMethods_[i].ml_flags = methods_[i]->getFlags();
        embMethods_[i].ml_doc = methods_[i]->getDesc2();
    }
    embMethods_[N].ml_name = nullptr;
    embMethods_[N].ml_meth = nullptr;
    embMethods_[N].ml_flags = 0;
    embMethods_[N].ml_doc = nullptr;
    return embMethods_;

}

void PyModule::addMethod(PyMethod* method) {
    methods_.push_back(method);
}

const char* PyModule::getModuleName() {return moduleName_.c_str();}

void PyModule::printInfo() {
    for (auto& elem : methods_) {
        std::string msg = "print(\"";
        msg += elem->getName() + ":\t";
        msg += elem->getDesc();
        msg += "\")\n";
        PyRun_SimpleString(msg.c_str());
    }
}

std::vector<PyMethod*> PyModule::getPyMethods() {
    return methods_;
}

PyModule* PyModule::getModuleByPyObject(PyObject* obj) {
    return instances_[obj];
}

void PyModule::setPyObject(PyObject* obj) {
    obj_ = obj;
    instances_[obj] = this;
}

PyObject* PyModule::getPyObject() {
    return obj_;
}




}//namespace