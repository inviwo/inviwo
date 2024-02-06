/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

#include <modules/oit/oitmoduledefine.h>  // for IVW_MODULE_MESHRENDERI...

#include <inviwo/core/util/staticstring.h>
#include <inviwo/core/ports/datainport.h>   // for DataInport
#include <inviwo/core/ports/dataoutport.h>  // for DataOutport
#include <modules/oit/datastructures/rasterization.h>

namespace inviwo {

class IVW_MODULE_OIT_API RasterizationInport : public DataInport<Rasterization, 0> {
public:
    using Super = DataInport<Rasterization, 0>;
    using Super::Super;

    virtual bool canConnectTo(const Port* port) const override;

    virtual void connectTo(Outport* port) override {
        if (!port->getConnectedInports().empty()) {
            throw Exception(IVW_CONTEXT,
                            "A RasterizationOutport can only be connected to one inport");
        }
        Super::connectTo(port);
    }
};

class IVW_MODULE_OIT_API RasterizationOutport : public DataOutport<Rasterization> {
public:
    using Super = DataOutport<Rasterization>;
    using Super::Super;
};

inline bool RasterizationInport::canConnectTo(const Port* port) const {
    if (!port || port->getProcessor() == getProcessor() || circularConnection(port)) {
        return false;
    }

    if (auto outport = dynamic_cast<const RasterizationOutport*>(port)) {
        if (outport->getConnectedInports().empty()) {
            // Only allow an outport to be connected to one inport
            return true;
        }
    }

    return false;
}

template <>
struct PortTraits<RasterizationInport> {
    static std::string_view classIdentifier() {
        static const std::string classId =
            fmt::format("{}.multi.inport", DataTraits<Rasterization>::classIdentifier());
        return classId;
    }
};

template <>
struct PortTraits<RasterizationOutport> {
    static std::string_view classIdentifier() {
        static const std::string classId =
            fmt::format("{}outport", DataTraits<Rasterization>::classIdentifier());
        return classId;
    }
};

}  // namespace inviwo
