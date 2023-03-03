/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2023 Inviwo Foundation
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

#include <modules/python3/pyutils.h>

#include <pybind11/cast.h>           // for object_api::operator(), object::cast
#include <pybind11/detail/common.h>  // for pybind11
#include <pybind11/pybind11.h>       // for module_, module
#include <pybind11/pytypes.h>        // for list, str_attr_accessor, object_api

#include <inviwo/core/util/exception.h>         // for Exception
#include <inviwo/core/util/sourcecontext.h>     // for IVW_CONTEXT_CUSTOM
#include <inviwo/core/util/stringconversion.h>  // for replaceInString

#include <array>        // for array
#include <string_view>  // for string_view

namespace inviwo {

namespace pyutil {

void addModulePath(std::string_view path) {
    namespace py = pybind11;

    if (!Py_IsInitialized()) {
        throw Exception("addModulePath(): Python is not initialized", IVW_CONTEXT_CUSTOM("pyutil"));
    }

    std::string pathConv{path};
    replaceInString(pathConv, "\\", "/");

    py::module::import("sys").attr("path").cast<py::list>().append(pathConv);
}

void removeModulePath(std::string_view path) {
    namespace py = pybind11;

    if (!Py_IsInitialized()) {
        throw Exception("removeModulePath(): Python is not initialized",
                        IVW_CONTEXT_CUSTOM("pyutil"));
    }

    std::string pathConv{path};
    replaceInString(pathConv, "\\", "/");

    py::module::import("sys").attr("path").attr("remove")(pathConv);
}

ModulePath::ModulePath(std::string_view path) : path_{path} { addModulePath(path_); }

ModulePath::~ModulePath() { removeModulePath(path_); }

}  // namespace pyutil

}  // namespace inviwo
