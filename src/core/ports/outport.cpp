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

#include <inviwo/core/ports/outport.h>
#include <inviwo/core/processors/processor.h>

namespace inviwo {

Outport::Outport(std::string identifier)
    : Port(identifier), invalidationLevel_(InvalidationLevel::Valid) {}

Outport::~Outport() {}

bool Outport::isConnected() const { return !(connectedInports_.empty()); }

bool Outport::isConnectedTo(const Inport* port) const {
    return util::contains(connectedInports_, port);
}

const std::vector<Inport*>& Outport::getConnectedInports() const { return connectedInports_; }

void Outport::invalidate(InvalidationLevel invalidationLevel) {
    invalidationLevel_ = invalidationLevel;
    for (auto port : connectedInports_) port->invalidate(invalidationLevel);
}

void Outport::propagateEvent(Event* event) {
    Processor* p = getProcessor();
    if (!event->hasVisitedProcessor(p)){
        event->markAsVisited(p);
        p->propagateEvent(event);
    }    
}

inviwo::InvalidationLevel Outport::getInvalidationLevel() const { return invalidationLevel_; }

void Outport::setValid() {
    invalidationLevel_ = InvalidationLevel::Valid;
    for (auto inport : connectedInports_) inport->setValid(this);
}

// Is called exclusively by Inport, which means a connection has been made.
void Outport::connectTo(Inport* inport) {
    util::push_back_unique(connectedInports_, inport);
}

// Is called exclusively by Inport, which means a connection has been removed.
void Outport::disconnectFrom(Inport* inport) {
    util::erase_remove(connectedInports_, inport);
}

}  // namespace
