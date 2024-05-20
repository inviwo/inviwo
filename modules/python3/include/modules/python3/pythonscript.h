/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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

#pragma once

#include <modules/python3/python3moduledefine.h>  // for IVW_MODULE_PYTHON3_API

#include <pybind11/pybind11.h>  // IWYU pragma: keep

#include <inviwo/core/util/callback.h>  // for BaseCallBack, CallBackList

#include <functional>     // for function
#include <string>         // for string
#include <unordered_map>  // for unordered_map
#include <filesystem>

namespace inviwo {

/**
 * Class for handling storage, compile and running of Python Scripts.
 * Used by PythonScriptEditor and PythonModule
 *
 */
class IVW_MODULE_PYTHON3_API PythonScript {

public:
    PythonScript(std::string_view source = "", std::string_view name = "");
    
    static PythonScript fromPath(const std::filesystem::path& path);

    /**
     * Frees the stored byte code. Make sure that the
     * Python interpreter is still initialized
     * when deleting the script.
     */
    ~PythonScript();

    /**
     * Sets the source for the Python (replacing the current source).
     */
    void setSource(std::string_view source);

    /**
     * Returns the script's source.
     */
    const std::string& getSource() const;

    void setName(std::string_view name);
    const std::string& getName() const;

    /**
     * Runs the script once.
     * If the script has changed since last compile a new compile call will be issued.
     *
     * If an error occurs, the error message is logged to the inviwo logger and python standard
     * output.
     *
     * @param locals a map of  keys and pybind11::object that will available as local variables in
     * the python scripts
     * @param callback a callback that will be called once the script has finished executing. The
     * callback takes a pybind11::dict) representing the python dict of local variables from the
     * script. Can be used to parse results from the script. This callback will only be called of
     * the script executed with out problems
     *
     * @return true, if script execution has been successful
     */
    bool run(std::unordered_map<std::string, pybind11::object> locals =
                 std::unordered_map<std::string, pybind11::object>{},
             std::function<void(pybind11::dict)> callback = nullptr);

    bool run(std::function<void(pybind11::dict)> callback);
    bool run(pybind11::dict locals, std::function<void(pybind11::dict)> callback = nullptr);

private:
    bool checkCompileError();
    bool checkRuntimeError();

    static std::string getAndFormatError();

    /**
     * Compiles the script source to byte code, which speeds up script execution. This function
     * is called by run when needed (eg. the source code has changed)
     *
     * @return true, if script compilation has been successful
     */
    bool compile();

    std::string source_;
    std::string name_;
    void* byteCode_;
    bool isCompileNeeded_;
};

}  // namespace inviwo
