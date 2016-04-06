/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/singleton.h>
#include <modules/python3/pythonexecutionoutputobservable.h>
#include <modules/python3/pythonincluder.h>

namespace inviwo {
class PyModule;
class Python3Module;

class IVW_MODULE_PYTHON3_API PyInviwo : public Singleton<PyInviwo>,
                                        public PythonExecutionOutputObservable {
public:
    PyInviwo(Python3Module* module);
    virtual ~PyInviwo();

    /**
    * \brief initialize a python module
    *
    * Initializes given python module definition.
    *
    * @param PyModuleDef *def definition of module
    * @param std::string name the name of the python module, will be used to access functions in the
    * script: eg inviwo.setPropertyValue(....)
    */
    void registerPyModule(PyModuleDef* def, std::string name);

    /**
    * \brief add a path for where python scripts will look for modules
    *
    * add a path for where python scripts will look for modules
    *
    * @param const std::string& path path to folder
    * @param PyMethodDef * module static array of registered classes in a module
    */
    void addModulePath(const std::string& path);

    std::vector<PyModule*> getAllPythonModules();

    void importModule(const std::string& moduleName);

protected:
    void initPythonCInterface(Python3Module* module);
    void initOutputRedirector(Python3Module* module);

private:
    bool isInit_;
    PyObject* dict_;

    std::vector<PyModule*> registeredModules_;
};

}  // namespace

#endif  // IVW_PYINVIWO_H
