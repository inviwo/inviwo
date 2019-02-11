/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#ifndef IVW_PORTINSPECTOR_H
#define IVW_PORTINSPECTOR_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/io/serialization/serializable.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/portconnection.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <string.h>

namespace inviwo {

class IVW_CORE_API PortInspector : public Serializable {
public:
    PortInspector();  // Should only be used for deserialization.
    PortInspector(std::string portClassIdentifier, std::string inspectorWorkspaceFileName);
    virtual ~PortInspector();

    const std::string& getInspectorNetworkFileName() const;
    const std::string& getPortClassName() const;

    const std::vector<Processor*>& getProcessors() const;
    const std::vector<Inport*>& getInports() const;
    CanvasProcessor* getCanvasProcessor() const;
    const std::vector<PortConnection>& getConnections() const;
    const std::vector<PropertyLink>& getPropertyLinks() const;

    virtual void serialize(Serializer& s) const override;

    virtual void deserialize(Deserializer& d) override;

private:
    std::string inspectorNetworkFileName_;
    std::string portClassIdentifier_;

    std::vector<Processor*> processors_;
    std::vector<Inport*> inports_;
    std::vector<PortConnection> connections_;
    std::vector<PropertyLink> propertyLinks_;
    CanvasProcessor* canvasProcessor_ = nullptr;
};

}  // namespace inviwo

#endif  // IVW_PORTINSPECTOR_H
