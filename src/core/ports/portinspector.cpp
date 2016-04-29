/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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
#include <inviwo/core/io/serialization/serialization.h>
#include <inviwo/core/metadata/processormetadata.h>

namespace inviwo {
PortInspector::PortInspector(std::string portClassIdentifier,
                             std::string inspectorWorkspaceFileName)
    : inspectorNetworkFileName_(inspectorWorkspaceFileName)
    , portClassIdentifier_(portClassIdentifier)
    , active_(false)
    , needsUpdate_(false)
    , inspectorNetwork_() {
    InviwoApplication::getPtr()->registerFileObserver(this);
    initialize();
}

PortInspector::~PortInspector() {
    InviwoApplication::getPtr()->unRegisterFileObserver(this);
}

void PortInspector::setActive(bool val) { active_ = val; }

bool PortInspector::isActive() { return active_; }

std::string PortInspector::getInspectorNetworkFileName() { return inspectorNetworkFileName_; }

std::string PortInspector::getPortClassName() { return portClassIdentifier_; }

std::vector<Inport*>& PortInspector::getInports() { return inPorts_; }

CanvasProcessor* PortInspector::getCanvasProcessor() { return canvasProcessor_; }

std::vector<PortConnection>& PortInspector::getConnections() { return connections_; }
std::vector<PropertyLink>& PortInspector::getPropertyLinks() { return propertyLinks_; }
std::vector<Processor*>& PortInspector::getProcessors() { return processors_; }

void PortInspector::fileChanged(const std::string& fileName) { needsUpdate_ = true; }

void PortInspector::initialize() {
    if (active_ == false && needsUpdate_) {
        if (inspectorNetwork_) {
            inspectorNetwork_.reset();
            processors_.clear();
            inPorts_.clear();
            connections_.clear();
            propertyLinks_.clear();
            canvasProcessor_ = nullptr;
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
        auto app = InviwoApplication::getPtr();
        Deserializer xmlDeserializer(app, inspectorNetworkFileName_);
        inspectorNetwork_ = util::make_unique<ProcessorNetwork>(app);
        inspectorNetwork_->deserialize(xmlDeserializer);
        processors_ = inspectorNetwork_->getProcessors();

        for (auto processor : processors_) {
            // Set Identifiers
            std::string newIdentifier =
                getPortClassName() + "_Port_Inspector_" + processor->getIdentifier();
            processor->setIdentifier(newIdentifier);
            // Find the and save inports.
            std::vector<Inport*> inports = processor->getInports();

            for (auto& inport : inports) {
                if (!inport->isConnected()) inPorts_.push_back(inport);
            }

            auto meta =
                processor->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
            meta->setVisibile(false);
            meta->setSelected(false);

            // Find and save the canvasProcessor
            if (auto canvasProcessor = dynamic_cast<CanvasProcessor*>(processor)) {
                canvasProcessor_ = canvasProcessor;
            }
        }

        // Store the connections and and disconnect them.
        auto connections = inspectorNetwork_->getConnections(); 
        for (auto& elem : connections) {
            connections_.emplace_back(elem.getOutport(), elem.getInport());
            inspectorNetwork_->removeConnection(elem.getOutport(), elem.getInport());
        }

        // store the processor links.
        propertyLinks_ = inspectorNetwork_->getLinks();

    } catch (AbortException& e) {
        // Error deserializing file
        needsUpdate_ = true;
        LogError(e.what());
    }
}

}  // namespace
