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

#pragma once

#include <modules/python3/python3moduledefine.h>
#include <inviwo/core/properties/scriptbackendfactory.h>

namespace inviwo {

class PyAnyConverter;

/**
 * @brief ScriptBackendFactoryObject that creates Python script backends.
 *
 * Uses the PyAnyConverter from the Python3Module for type conversions between
 * std::any and pybind11::object. The created backends execute Python scripts
 * using pybind11::exec and return results via the `__result__` convention.
 *
 * @see ScriptBackendFactoryObject
 * @see PyAnyConverter
 * @see ScriptProperty
 */
class IVW_MODULE_PYTHON3_API PythonScriptBackendFactoryObject : public ScriptBackendFactoryObject {
public:
    /**
     * @param converter Reference to the PyAnyConverter for type conversions.
     *        Must remain valid for the lifetime of any backends created by this factory object.
     */
    explicit PythonScriptBackendFactoryObject(const PyAnyConverter& converter);

    virtual ScriptProperty::Backend create() const override;

private:
    const PyAnyConverter& converter_;
};

}  // namespace inviwo
