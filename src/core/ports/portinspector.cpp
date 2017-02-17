/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2017 Inviwo Foundation
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

namespace inviwo {

PortInspector::PortInspector() : network_(InviwoApplication::getPtr()) {

}

PortInspector::PortInspector(std::string portClassIdentifier,
                             std::string inspectorWorkspaceFileName)
    : inspectorNetworkFileName_(inspectorWorkspaceFileName)
    , portClassIdentifier_(portClassIdentifier)
    , network_(InviwoApplication::getPtr()) {

    // Deserialize the network
    if (auto istream = std::ifstream(inspectorNetworkFileName_)) {
        auto app = InviwoApplication::getPtr();
        auto deserializer = app->getWorkspaceManager()->createWorkspaceDeserializer(
            istream, inspectorNetworkFileName_);
        deserializer.deserialize("ProcessorNetwork", network_);

        for (auto processor : network_.getProcessors()) {
            // Set Identifiers
            std::string newIdentifier = dotSeperatedToPascalCase(getPortClassName()) +
                                        "PortInspector" + processor->getIdentifier();
            processor->setIdentifier(newIdentifier);

            // Find the and save inports.
            for (auto& inport : processor->getInports()) {
                if (!inport->isConnected()) inPorts_.push_back(inport);
            }

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

        // Store the connections and and disconnect them.
        auto connections = network_.getConnections();
        for (auto& connection : connections) {
            connections_.emplace_back(connection.getOutport(), connection.getInport());
            network_.removeConnection(connection.getOutport(), connection.getInport());
        }

        // store the processor links.
        propertyLinks_ = network_.getLinks();
        
    } else {
        throw Exception("Could not open port inspector file: " + inspectorNetworkFileName_,
                        IvwContext);
    }
}

const std::string& PortInspector::getInspectorNetworkFileName() const {
    return inspectorNetworkFileName_;
}
const std::string& PortInspector::getPortClassName() const { return portClassIdentifier_; }
const std::vector<Inport*>& PortInspector::getInports() const { return inPorts_; }
CanvasProcessor* PortInspector::getCanvasProcessor() const { return canvasProcessor_; }
const std::vector<PortConnection>& PortInspector::getConnections() const { return connections_; }
const std::vector<PropertyLink>& PortInspector::getPropertyLinks() const { return propertyLinks_; }
std::vector<Processor*> PortInspector::getProcessors() const {
    return network_.getProcessors();
}

void PortInspector::serialize(Serializer& s) const {
    s.serialize("portClassIdentifier", portClassIdentifier_);
    s.serialize("inspectorNetworkFileName", inspectorNetworkFileName_);
    s.serialize("network", network_);
    s.serialize("inports", inPorts_, "inport");
    s.serialize("connections", connections_, "connection");
    s.serialize("links", propertyLinks_, "link");
    s.serialize("canvas", canvasProcessor_);
}

void PortInspector::deserialize(Deserializer& d) {
    d.deserialize("portClassIdentifier", portClassIdentifier_);
    d.deserialize("inspectorNetworkFileName", inspectorNetworkFileName_);
    d.deserialize("network", network_);
    d.deserialize("inports", inPorts_, "inport");
    d.deserialize("connections", connections_, "connection");
    d.deserialize("links", propertyLinks_, "link");
    d.deserialize("canvas", canvasProcessor_);
}

}  // namespace
