/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <inviwo/core/ports/outport.h>

#include <inviwo/core/processors/processor.h>

namespace inviwo {

Outport::Outport(std::string identifier, InvalidationLevel invalidationLevel)
    : Port(identifier), invalidationLevel_(invalidationLevel)
{}

Outport::~Outport() {}

bool Outport::isConnected() const {
    return !(connectedInports_.empty());
}

bool Outport::isConnectedTo(Inport* port) const {
    return !(std::find(connectedInports_.begin(),connectedInports_.end(),port)==connectedInports_.end());
}

void Outport::invalidate(InvalidationLevel invalidationLevel) {
    invalidationLevel_ = invalidationLevel;
    invalidateConnectedInports(invalidationLevel_);
}

void Outport::invalidateConnectedInports(InvalidationLevel invalidationLevel){
    for (size_t i=0; i<connectedInports_.size(); i++)
        connectedInports_[i]->invalidate(invalidationLevel);
}

void Outport::setInvalidationLevel(InvalidationLevel invalidationLevel){
    invalidationLevel_ = invalidationLevel;
    for (size_t i=0; i<connectedInports_.size(); i++)
        connectedInports_[i]->setInvalidationLevel(invalidationLevel);
}

template <typename T>
void Outport::getSuccessorsUsingPortType(std::vector<Processor*>& successorProcessors) {
    for (size_t i=0; i<connectedInports_.size(); i++) {
        Processor* decendantProcessor = connectedInports_[i]->getProcessor();

        if (std::find(successorProcessors.begin(), successorProcessors.end(), decendantProcessor)== successorProcessors.end())
            successorProcessors.push_back(connectedInports_[i]->getProcessor());

        std::vector<Outport*> outports = decendantProcessor->getOutports();

        for (size_t j=0; j<outports.size(); j++) {
            T* outPort = dynamic_cast<T*>(outports[j]);

            if (outPort)
                outPort->template getSuccessorsUsingPortType<T>(successorProcessors);
        }
    }
}

std::vector<Processor*> Outport::getDirectSuccessors() {
    std::vector<Processor*> successorProcessors;
    getSuccessorsUsingPortType<Outport>(successorProcessors);
    return successorProcessors;
}

//Is called exclusively by Inport, which means a connection has been made.
void Outport::connectTo(Inport* inport) {
    if (std::find(connectedInports_.begin(), connectedInports_.end(), inport) == connectedInports_.end()) {
        connectedInports_.push_back(inport);
    }
}

//Is called exclusively by Inport, which means a connection has been removed.
void Outport::disconnectFrom(Inport* inport) {
    if (std::find(connectedInports_.begin(), connectedInports_.end(), inport) != connectedInports_.end()) {
        connectedInports_.erase(std::remove(connectedInports_.begin(), connectedInports_.end(), inport),
                                connectedInports_.end());
    }
}


} // namespace
