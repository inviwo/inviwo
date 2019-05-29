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

#include <inviwo/core/ports/portinspector.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/io/serialization/serialization.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/core/util/filesystem.h>

namespace inviwo {

PortInspector::PortInspector() {}

PortInspector::PortInspector(std::string portClassIdentifier,
                             std::string inspectorWorkspaceFileName)
    : inspectorNetworkFileName_(inspectorWorkspaceFileName)
    , portClassIdentifier_(portClassIdentifier) {

    // Deserialize the network
    if (auto istream = filesystem::ifstream(inspectorNetworkFileName_)) {
        auto app = InviwoApplication::getPtr();
        auto deserializer = app->getWorkspaceManager()->createWorkspaceDeserializer(
            istream, inspectorNetworkFileName_);

        ProcessorNetwork network(app);
        deserializer.deserialize("ProcessorNetwork", network);
        processors_ = network.getProcessors();

        for (auto processor : processors_) {
            // Set Identifiers
            std::string newIdentifier = dotSeperatedToPascalCase(getPortClassName()) +
                                        "PortInspector" + processor->getIdentifier();
            processor->setIdentifier(newIdentifier);

            // Find the and save inports.
            std::copy_if(processor->getInports().begin(), processor->getInports().end(),
                         std::back_inserter(inports_),
                         [](const auto& inport) { return !inport->isConnected(); });

            auto meta =
                processor->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
            meta->setVisible(false);
            meta->setSelected(false);

            // Find and save the canvasProcessor
            if (auto canvasProcessor = dynamic_cast<CanvasProcessor*>(processor)) {
                canvasProcessor_ = canvasProcessor;
                // Mark the widget as a "PortInspector"
                auto md = canvasProcessor->createMetaData<BoolMetaData>("PortInspector");
                md->set(true);
            }
        }

        // Store the connections.
        connections_ = network.getConnections();
        // Store the processor links.
        propertyLinks_ = network.getLinks();

        // Manually clear the network so out processors won't get deleted.
        for (auto processor : processors_) {
            network.removeProcessor(processor);
        }

        if (!canvasProcessor_) {
            throw Exception(
                "Could not find canvas for port inspector: " + inspectorNetworkFileName_,
                IVW_CONTEXT);
        }

    } else {
        throw Exception("Could not open port inspector file: " + inspectorNetworkFileName_,
                        IVW_CONTEXT);
    }
}

PortInspector::~PortInspector() {
    for (auto processor : processors_) {
        delete processor;
    }
}

const std::string& PortInspector::getInspectorNetworkFileName() const {
    return inspectorNetworkFileName_;
}
const std::string& PortInspector::getPortClassName() const { return portClassIdentifier_; }
const std::vector<Inport*>& PortInspector::getInports() const { return inports_; }
CanvasProcessor* PortInspector::getCanvasProcessor() const { return canvasProcessor_; }
const std::vector<PortConnection>& PortInspector::getConnections() const { return connections_; }
const std::vector<PropertyLink>& PortInspector::getPropertyLinks() const { return propertyLinks_; }
const std::vector<Processor*>& PortInspector::getProcessors() const { return processors_; }

void PortInspector::serialize(Serializer& s) const {
    s.serialize("PortClassIdentifier", portClassIdentifier_);
    s.serialize("InspectorNetworkFileName", inspectorNetworkFileName_);
    s.serialize("Processors", processors_, "Processor");
    s.serialize("Inports", inports_, "Inport");
    s.serialize("Connections", connections_, "Connection");
    s.serialize("Links", propertyLinks_, "Link");
    s.serialize("Canvas", canvasProcessor_);
}

void PortInspector::deserialize(Deserializer& d) {
    d.deserialize("PortClassIdentifier", portClassIdentifier_);
    d.deserialize("InspectorNetworkFileName", inspectorNetworkFileName_);
    d.deserialize("Processors", processors_, "Processor");
    d.deserialize("Inports", inports_, "Inport");
    d.deserialize("Connections", connections_, "Connection");
    d.deserialize("Links", propertyLinks_, "Link");
    d.deserialize("Canvas", canvasProcessor_);
}

}  // namespace inviwo
