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

#include <modules/python3/pythoninterface/pymethod.h>
#include <modules/python3/pythoninterface/pymodule.h>
#include <inviwo/core/util/assertion.h>
#include <sstream>


namespace inviwo {

PyParamBase::PyParamBase(std::string paramName,bool optional)
    : name_(paramName)
    , optional_(optional)
{
}

std::string PyParamBase::getParamName()const {
    return name_;
}

bool PyParamBase::isOptional()const {
    return optional_;
}

PyMethod::PyMethod() :optionalParams_(0) {
    def_ = new PyMethodDef();
}

PyMethod::~PyMethod() {
    delete def_;
}


PyMethodDef* PyMethod::getDef() {
    def_->ml_doc   = getDesc2();
    def_->ml_flags = getFlags();
    def_->ml_meth  = getFunc();
    def_->ml_name  = getName2();
    return def_;
}

void PyMethod::setDef(PyMethodDef &def){
    def.ml_doc = getDesc2();
    def.ml_flags = getFlags();
    def.ml_meth = getFunc();
    def.ml_name = getName2();
}

std::string PyMethod::getParamDesc() {
    if (params_.empty()) {
        return "None";
    }

    std::stringstream ss;

    for (size_t i = 0; i<params_.size(); i++) {
        if (i!=0)
            ss << " , ";

        if (params_[i]->isOptional())
            ss << "[";

        ss << params_[i]->paramType() << " " << params_[i]->getParamName();

        if (params_[i]->isOptional())
            ss << "]";
    }

    return ss.str();
}

bool PyMethod::testParams(PyObject* args)const {
    size_t size = static_cast<size_t>(PyTuple_Size(args));

    if (size > params_.size() || size < (params_.size()-optionalParams_)) {
        std::stringstream ss;
        ss << getName() << "() expects " << params_.size()-optionalParams_ << " parameters ";

        if (optionalParams_!=0) {
            ss << " and may have " << optionalParams_ << " additional parameters";
        }

        PyErr_SetString(PyExc_TypeError,ss.str().c_str());
        return false;
    }

    for (size_t i = 0; i<size; i++) {
        if (!params_[i]->testParam(PyTuple_GetItem(args,i))) {
            std::stringstream ss;
            ss << getName() << "() expects a " << params_[i]->paramType() << " for its " << i+1 << ":th parameter";
            PyErr_SetString(PyExc_TypeError,ss.str().c_str());
            return false;
        }
    }

    return true;
}

void PyMethod::addParam(PyParamBase* param) {
    ivwAssert(optionalParams_ == 0 || (optionalParams_ != 0
                                       && param->isOptional()),"Once one paramter has been marked as optional, the rest of the has to as well");
    params_.push_back(param);

    if (param->isOptional())
        optionalParams_++;
}

int PyMethod::getFlags(){
    return METH_VARARGS;
}




}//namespace