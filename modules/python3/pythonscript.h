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

#ifndef IVW_PYTHONSCRIPT_H
#define IVW_PYTHONSCRIPT_H

#include <modules/python3/python3moduledefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

    /**
    * Class for handling storage, compile and running of Python Scripts.
    * Used by PythonScriptEditor and PythonModule
    *
    */
    class IVW_MODULE_PYTHON3_API PythonScript {

    public:
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
        * If an error occurs, the error message is logged to the inviwo logger and python standard output.
        *
        * @return true, if script execution has been successful
        */
        bool run(bool outputInfo = true);

    private:
        bool checkCompileError();
        bool checkRuntimeError();

        /**
        * Compiles the script source to byte code, which speeds up script execution
        * and is generally recommended, since it also produces more clear error messages.
        *
        * If an error occurs, the error message is stored and can be retrieved through getLog().
        *
        * @param logErrors if true, error messages are also passed to the logger.
        *  The internal log buffer is not affected by this parameter.
        *
        * @return true, if script compilation has been successful
        */
        bool compile(bool outputInfo = true);

        std::string source_;
        void* byteCode_;
        bool isCompileNeeded_;
    };

} // namespace

#endif // IVW_PYTHONSCRIPT_H

