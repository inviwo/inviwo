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

#ifndef IVW_PYMEHTODINVIWO_H
#define IVW_PYMEHTODINVIWO_H

#include <modules/python3/python3moduledefine.h>
#include <modules/python3/pythoninterface/pyvalueparser.h>

#include <vector>
#include <string>

namespace inviwo {
class IVW_MODULE_PYTHON3_API PyParamBase {
public:
    PyParamBase(std::string paramName,bool optional);
    virtual ~PyParamBase() {}
    std::string getParamName()const;
    bool isOptional()const;
    virtual bool testParam(void*) = 0;
    virtual std::string paramType()const = 0;
private:
    #include <warn/push>
    #include <warn/ignore/dll-interface>
    std::string name_;
    #include <warn/pop>
    bool optional_;
};

template<typename T>
class PyParam : public PyParamBase {
public:
    PyParam(std::string paramName,bool optional) : PyParamBase(paramName,optional) {}
    virtual ~PyParam() {}
    virtual bool testParam(void* arg){
        return PyValueParser::is<T>(static_cast<PyObject*>(arg));
    }
};

#define PY_PARAM(T,t,n) class IVW_MODULE_PYTHON3_API PyParam##t : public PyParam<T>{ \
    public:\
        PyParam##t(std::string paramName,bool optional = false) :PyParam<T>(paramName,optional){} \
        virtual ~PyParam##t(){}\
        virtual std::string paramType()const{return n;}\
    };

PY_PARAM(std::string,String,"string")
PY_PARAM(int,Int,"int")
PY_PARAM(float,Float,"int")
PY_PARAM(vec2,Vec2,"vec2")
PY_PARAM(vec3,Vec3,"vec3")
PY_PARAM(vec4,Vec4,"vec4")
PY_PARAM(uvec2,UVec2,"uvec2")
PY_PARAM(uvec3,UVec3,"uvec3")
PY_PARAM(uvec4,UVec4,"uvec4")


class IVW_MODULE_PYTHON3_API PyParamVarious : public PyParamBase {
public:
    PyParamVarious(std::string paramName,bool optional = false) : PyParamBase(paramName,optional) {}
    virtual ~PyParamVarious() {}
    virtual bool testParam(void* arg) {
        return true;
    }
    virtual std::string paramType()const {return "Various";}
};

class IVW_MODULE_PYTHON3_API PyMethod {
public:
    PyMethod();
    virtual ~PyMethod();
    virtual std::string getName()const = 0;
    virtual std::string getDesc()const = 0;
    virtual const char * getName2(){
        if (name_.size() == 0){
            name_ = getName();
        }
        return name_.c_str();
    }
    virtual const char * getDesc2(){
        if (desc_.size() == 0){
            desc_ = getDesc();
        }
        return desc_.c_str();
    }
    virtual std::string getParamDesc();
    virtual PyCFunction getFunc() = 0;
    virtual int getFlags();

    PyMethodDef* getDef();
    void setDef(PyMethodDef&);

    bool testParams(PyObject* args)const;

protected:
    void addParam(PyParamBase* param);
    PyMethodDef* def_;
    std::vector<PyParamBase*> params_;
    std::string name_;
    std::string desc_;
    size_t optionalParams_;

};
} //namespace

#endif // IVW_PYMEHTODINVIWO_H
