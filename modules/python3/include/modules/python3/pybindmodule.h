/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2025 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <warn/pop>

// based on pybind11/embed.h PYBIND11_EMBEDDED_MODULE
// But defines a function
// void inviwo_static_pybind_init_<name>()
// instead of using a global static variable to initialize the module
// we need this to control the initialization order

#ifdef INVIWO_ALL_DYN_LINK
#define INVIWO_PYBIND_MODULE(name, variable) PYBIND11_MODULE(name, variable)
#else
#define INVIWO_PYBIND_MODULE(name, variable)                                                 \
    PYBIND11_MODULE_PYINIT(name, {})                                                         \
    void PYBIND11_CONCAT(inviwo_static_pybind_init_, name)() {                               \
        if (Py_IsInitialized() != 0) {                                                       \
            pybind11::pybind11_fail("Can't add new module '" PYBIND11_TOSTRING(              \
                name) "' after the interpreter has been initialized");                       \
        }                                                                                    \
        auto result =                                                                        \
            PyImport_AppendInittab(PYBIND11_TOSTRING(name), PYBIND11_CONCAT(PyInit_, name)); \
        if (result == -1) {                                                                  \
            pybind11::pybind11_fail(                                                         \
                "Insufficient memory to add a new module '" PYBIND11_TOSTRING(name) "'");    \
        }                                                                                    \
    }                                                                                        \
    PYBIND11_MODULE_EXEC(name, variable)
#endif
