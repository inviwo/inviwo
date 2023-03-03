/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2023 Inviwo Foundation
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

#include <pybind11/pybind11.h>  /// IWYU pragma: keep

#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/porttraits.h>
#include <inviwo/core/util/glmvec.h>

namespace inviwo {

class IVW_MODULE_PYTHON3_API PythonInport : public Inport {
public:
    PythonInport(std::string_view identifier, Document help = {});
    virtual ~PythonInport() = default;

    virtual std::string getClassIdentifier() const override;
    virtual uvec3 getColorCode() const override { return uvec3{12, 240, 153}; }
    virtual Document getInfo() const override;
    virtual size_t getMaxNumberOfConnections() const override { return 1; }

    virtual bool canConnectTo(const Port* port) const override;

    pybind11::object getData() const;
    bool hasData() const;
};

template <>
struct PortTraits<PythonInport> {
    static const std::string& classIdentifier() {
        static std::string id{"org.inviwo.pythoninport"};
        return id;
    }
};

inline std::string PythonInport::getClassIdentifier() const {
    return PortTraits<PythonInport>::classIdentifier();
}

}  // namespace inviwo
