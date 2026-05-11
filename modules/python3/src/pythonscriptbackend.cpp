/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/python3/pythonscriptbackend.h>

#include <modules/python3/pyanyconverter.h>

#include <inviwo/core/util/exception.h>

#include <pybind11/pybind11.h>
#include <pybind11/eval.h>

namespace inviwo {

PythonScriptBackendFactoryObject::PythonScriptBackendFactoryObject(const PyAnyConverter& converter)
    : ScriptBackendFactoryObject("python"), converter_{converter} {}

ScriptProperty::Backend PythonScriptBackendFactoryObject::create() const {
    return [&converter = converter_](const std::string& source,
                                     const std::vector<std::any>& args) -> std::any {
        namespace py = pybind11;
        const py::gil_scoped_acquire guard{};

        py::dict globals = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));

        py::list pyArgs;
        for (const auto& arg : args) {
            pyArgs.append(converter.toPyObject(arg));
        }
        globals["__args__"] = pyArgs;

        try {
            py::exec(source, globals, globals);
        } catch (const py::error_already_set& e) {
            throw Exception(IVW_CONTEXT_CUSTOM("PythonScriptBackend"),
                            "Python script error: {}", e.what());
        }

        if (globals.contains("__result__")) {
            return converter.toAny(globals["__result__"]);
        }

        return std::any{};
    };
}

}  // namespace inviwo
