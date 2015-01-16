/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_PYINVIWO_H
#define IVW_PYINVIWO_H

#include <modules/python3/python3moduledefine.h>
#include <modules/python3/pyinviwoobserver.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/singleton.h>
#include <inviwo/core/util/observer.h>


#ifndef PyObject_HEAD
struct PyObject;
#endif

namespace inviwo {
    class PyModule;
    class IVW_MODULE_PYTHON3_API PyInviwo : public Singleton<PyInviwo> , public Observable<PyInviwoObserver> {
public:
    PyInviwo();
    virtual ~PyInviwo();

    /**
    * \brief initialized python modules
    *
    * Initializes given python module. Can be used called from outside this python module.
    *
    * @param PyModule *pyModule class containing information and methods for python module
    */
    void registerPyModule(PyModule* pyModule);

    /**
    * \brief add a path for where ptyhon scripts will look for modules
    *
    * add a path for where ptyhon scripts will look for modules
    *
    * @param const std::string& path path to folder
    * @param PyMethodDef * module static array of registered classes in a module
    */
    void addModulePath(const std::string& path);
    PyObject* getMainDictionary(){ return mainDict_; }
    PyObject* getModulesDictionary(){ return modulesDict_; }

    std::vector<PyModule*> getAllPythonModules(){ return registeredModules_; }


    void importModule(const std::string &moduleName);
protected:
    void initPythonCInterface();
    void initDefaultInterfaces();
    void initOutputRedirector();
private:
    bool isInit_;
    PyModule* inviwoPyModule_;
    PyModule* inviwoInternalPyModule_;

    PyObject *mainDict_;
    PyObject *modulesDict_;

    std::vector<PyModule*> registeredModules_;
};

} // namespace

#endif // IVW_PYINVIWO_H

