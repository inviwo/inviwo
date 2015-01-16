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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/ports/portinspector.h>
#include <inviwo/core/io/serialization/ivwserialization.h>
#include <inviwo/core/metadata/processormetadata.h>

namespace inviwo {
PortInspector::PortInspector(std::string portClassIdentifier, std::string inspectorWorkspaceFileName)
    : inspectorNetworkFileName_(inspectorWorkspaceFileName)
    , portClassIdentifier_(portClassIdentifier)
    , active_(false)
    , needsUpdate_(false)
    , inspectorNetwork_(NULL) {
    InviwoApplication::getPtr()->registerFileObserver(this);
}

PortInspector::~PortInspector() {
    if (inspectorNetwork_)
        delete inspectorNetwork_;
}

void PortInspector::setActive(bool val) {
    initialize();
    active_ = val;
}

bool PortInspector::isActive() {
    return active_;
}

std::string PortInspector::getInspectorNetworkFileName() {
    return inspectorNetworkFileName_;
}

std::string PortInspector::getPortClassName() {
    return portClassIdentifier_;
}

std::vector<Inport*> PortInspector::getInports() {
    initialize();
    return inPorts_;
}

CanvasProcessor* PortInspector::getCanvasProcessor() {
    initialize();
    return canvasProcessor_;
}

std::vector<PortConnection*>  PortInspector::getConnections() {
    initialize();
    return connections_;
}
std::vector<PropertyLink*> PortInspector::getPropertyLinks() {
    initialize();
    return propertyLinks_;
}
std::vector<Processor*> PortInspector::getProcessors() {
    initialize();
    return processors_;
}

void PortInspector::fileChanged(std::string fileName) {
    needsUpdate_ = true;
}

void PortInspector::initialize() {
    if (active_ == false && needsUpdate_) {
        if (inspectorNetwork_) {
            delete inspectorNetwork_;
            inspectorNetwork_ = NULL;
            processors_.clear();
            inPorts_.clear();
            connections_.clear();
            propertyLinks_.clear();
            canvasProcessor_ = NULL;
        }

        stopFileObservation(inspectorNetworkFileName_);
        needsUpdate_ = false;
    }

    if (inspectorNetwork_) {
        return;
    }

    // Observe the filename;
    startFileObservation(inspectorNetworkFileName_);
    try {
        // Deserialize the network
        IvwDeserializer xmlDeserializer(inspectorNetworkFileName_);
        inspectorNetwork_ = new ProcessorNetwork();
        inspectorNetwork_->deserialize(xmlDeserializer);
        processors_ = inspectorNetwork_->getProcessors();

        for (size_t i = 0; i < processors_.size(); i++) {
            Processor* processor = processors_[i];
            // Set Identifiers
            std::string newIdentifier =
                getPortClassName() + "_Port_Inspector_" + processor->getIdentifier();
            processor->setIdentifier(newIdentifier);
            processor->initialize();
            // Find the and save inports.
            std::vector<Inport*> inports = processor->getInports();

            for (size_t i = 0; i < inports.size(); i++) {
                if (!inports[i]->isConnected()) inPorts_.push_back(inports[i]);
            }

            ProcessorMetaData* meta = processor->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
            meta->setVisibile(false);
            meta->setSelected(false);
           

            // Find and save the canvasProcessor
            CanvasProcessor* canvasProcessor = dynamic_cast<CanvasProcessor*>(processor);

            if (canvasProcessor) {
                canvasProcessor_ = canvasProcessor;
            }
        }

        // Store the connections and and disconnect them.
        connections_ = inspectorNetwork_->getConnections();

        for (size_t i = 0; i < connections_.size(); i++)
            connections_[i]->getInport()->disconnectFrom(connections_[i]->getOutport());

        // store the processor links.
        propertyLinks_ = inspectorNetwork_->getLinks();
    } catch (AbortException& e) {
        // Error deserializing file
        needsUpdate_ = true;
        LogError(e.what());
    }
}

PortInspectorFactoryObject::PortInspectorFactoryObject(const std::string& portClassIdentifier,
                               const std::string& inspectorWorkspaceFileName)
    : portClassIdentifier_(portClassIdentifier)
    , inspectorWorkspaceFileName_(inspectorWorkspaceFileName) {}

std::string PortInspectorFactoryObject::getClassIdentifier() const {
    return portClassIdentifier_;
}

PortInspector* PortInspectorFactoryObject::create() {
    return new PortInspector(portClassIdentifier_, inspectorWorkspaceFileName_);
}

} // namespace

