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

#ifndef IVW_PORTINSPECTOR_H
#define IVW_PORTINSPECTOR_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/portconnection.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/util/fileobserver.h>
#include <string.h>

namespace inviwo {

class IVW_CORE_API PortInspector : public FileObserver {
public:
    PortInspector(std::string portClassIdentifier, std::string inspectorWorkspaceFileName);
    virtual ~PortInspector();

    std::string getInspectorNetworkFileName();
    std::string getPortClassName();
    void setActive(bool val);
    bool isActive();

    std::vector<Processor*> getProcessors();
    std::vector<Inport*> getInports();
    CanvasProcessor* getCanvasProcessor();
    std::vector<PortConnection*> getConnections();
    std::vector<PropertyLink*> getPropertyLinks();

private:
    void initialize();

    std::string inspectorNetworkFileName_;
    std::string portClassIdentifier_;
    bool active_;
    bool needsUpdate_;
    ProcessorNetwork* inspectorNetwork_;

    std::vector<Processor*> processors_;
    std::vector<Inport*> inPorts_;
    std::vector<PortConnection*> connections_;
    std::vector<PropertyLink*> propertyLinks_;
    CanvasProcessor* canvasProcessor_;

    virtual void fileChanged(std::string fileName);
};

class IVW_CORE_API PortInspectorFactoryObject {
public:
    PortInspectorFactoryObject(const std::string& portClassIdentifier,
                               const std::string& inspectorWorkspaceFileName);
    virtual ~PortInspectorFactoryObject() {}

    virtual PortInspector* create();

    std::string getClassIdentifier() const;

private:
    std::string portClassIdentifier_;
    std::string inspectorWorkspaceFileName_;
};

}  // namespace

#endif  // IVW_PORTINSPECTOR_H
