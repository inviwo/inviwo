/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2017 Inviwo Foundation
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

#ifndef IVW_PYTHONSCRIPT_H
#define IVW_PYTHONSCRIPT_H

#include <modules/python3/python3moduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/python3/pythonincluder.h>
#include <inviwo/core/util/singlefileobserver.h>
#include <modules/python3/pythonincluder.h>

#include <unordered_map>

namespace inviwo {

    /**
    * Class for handling storage, compile and running of Python Scripts.
    * Used by PythonScriptEditor and PythonModule
    *
    */
    class IVW_MODULE_PYTHON3_API PythonScript {

    public:
        using VariableMap = std::unordered_map<std::string, PyObject*>;

        PythonScript();

        /**
        * Frees the stored byte code. Make sure that the
        * Python interpreter is still initialized
        * when deleting the script.
        */
        ~PythonScript();

        /**
        * Sets the source for the Python (replacing the current source).
        */
        void setSource(const std::string& source);

        /**
        * Returns the script's source.
        */
        std::string getSource() const;

        /**
        * Runs the script once,
        * if the script has changed since last compile a new compile call will be issued.
        *
        * If an error occurs, the error message is logged to the inviwo logger and python standard
        output.
        *
        * @param extraLocalVariables a map of  keys and PyOjbects that will available as local
        variables in the python scripts, eg {"number" , PyValueParser::toPyObject<int>(123) } will
        be avaible as the local variable 'number' in the script
        * @param callback a callback that will be called once the script has finished executing. The
        callback takes a PyObject representing the python dict of local variables from the script.
        Can be used to parse results from the script. This callback will only be called of the
        script executed with out problems
        *
        * @return true, if script execution has been successful
        */
        bool run(const VariableMap& extraLocalVariables = VariableMap(),
                 std::function<void(PyObject*)> callback = [](PyObject* obj) {});

        void setFilename(std::string filename);

    private:
        bool checkCompileError();
        bool checkRuntimeError();

        /**
        * Compiles the script source to byte code, which speeds up script execution. This function
        * is called by ::run when needed (eg. the source code has changed)
        *
        * @return true, if script compilation has been successful
        */
        bool compile();

        std::string source_;
        std::string filename_;
        void* byteCode_;
        bool isCompileNeeded_;
    };

    /**
    * Class for handling PythonScripts that exists as files on disk. PythonScriptDisk will observe
    * the file and reload it from disk when it detects it has changed. 
    *
    * @see PythonScript
    * @see SingleFileObserver
    */
    class IVW_MODULE_PYTHON3_API PythonScriptDisk : public PythonScript , public SingleFileObserver {
    public:
        PythonScriptDisk(std::string filename);
    private:
        void readFileAndSetSource();
    };

} // namespace

#endif // IVW_PYTHONSCRIPT_H

