/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/processors/processor.h>

namespace inviwo {

Inport::Inport(std::string identifier)
    : Port(identifier), changed_(false)
{}

Inport::~Inport() {}

bool Inport::isConnected() const { 
    return false; 
}

bool Inport::isReady() const { 
    return isConnected() && getConnectedOutport()->isValid(); 
}

void Inport::invalidate(InvalidationLevel invalidationLevel) {
    Port::invalidate(invalidationLevel);
}

std::vector<Processor*> Inport::getPredecessors() {
    std::vector<Processor*> predecessorsProcessors;
    getPredecessorsUsingPortType<Inport>(predecessorsProcessors);
    return predecessorsProcessors;
}

template <typename T>
void Inport::getPredecessorsUsingPortType(std::vector<Processor*>& predecessorsProcessors) {
    if (isConnected()) {
        std::vector<Outport*> connectedOutports = getConnectedOutports();
        std::vector<Outport*>::const_iterator it = connectedOutports.begin();
        std::vector<Outport*>::const_iterator endIt = connectedOutports.end();

        for (; it != endIt; ++it) {
            Processor* predecessorsProcessor = (*it)->getProcessor();

            if (std::find(predecessorsProcessors.begin(), predecessorsProcessors.end(), predecessorsProcessor)== predecessorsProcessors.end())
                predecessorsProcessors.push_back(predecessorsProcessor);

            std::vector<Inport*> inports = predecessorsProcessor->getInports();

            for (size_t j=0; j<inports.size(); j++) {
                T* inPort = dynamic_cast<T*>(inports[j]);

                if (inPort)
                    inPort->template getPredecessorsUsingPortType<T>(predecessorsProcessors);
            }
        }
    }
}

void Inport::setChanged(bool changed) { 
    changed_ = changed; 
}

void Inport::callOnChangeIfChanged() {
    if (isChanged()){
        onChangeCallback_.invokeAll();
    }
}

bool Inport::isChanged(){
    return changed_;
}

} // namespace
