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

#include <inviwo/core/ports/singleinport.h>

#include <inviwo/core/processors/processor.h>

namespace inviwo {

SingleInport::SingleInport(std::string identifier,
                           InvalidationLevel invalidationLevel)
    : Inport(identifier)
    , connectedOutport_(NULL)
    , invalidationLevel_(invalidationLevel) {
}

SingleInport::~SingleInport() {}

//Inport should determine if we can connect to the outport
void SingleInport::connectTo(Outport* outport) {
    connectedOutport_ = outport;
    outport->connectTo(this);
    changed_ = true;
    invalidate(INVALID_OUTPUT);
}

void SingleInport::disconnectFrom(Outport* outport) {
    if (outport == connectedOutport_) {
        onInvalidCallback_.invokeAll();
        connectedOutport_ = NULL;
        outport->disconnectFrom(this);
        changed_ = true;
        invalidate(INVALID_OUTPUT);
    }
}

bool SingleInport::isConnected() const {
    return (connectedOutport_!=NULL);
}

bool SingleInport::isConnectedTo(Outport* outport) const {
    return connectedOutport_==outport;
}

InvalidationLevel SingleInport::getInvalidationLevel() const{
    return invalidationLevel_;
}

void SingleInport::setInvalidationLevel(InvalidationLevel invalidationLevel){
    invalidationLevel_ = invalidationLevel;
    setChanged();
}

void SingleInport::invalidate(InvalidationLevel invalidationLevel) {
    if(getInvalidationLevel() == VALID && invalidationLevel >= INVALID_OUTPUT)
        onInvalidCallback_.invokeAll();
    invalidationLevel_ = std::max(invalidationLevel_, invalidationLevel);
    Inport::invalidate(invalidationLevel);
}

} // namespace
