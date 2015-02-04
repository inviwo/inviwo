/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <inviwo/core/network/portconnection.h>

namespace inviwo {

PortConnection::PortConnection() : inport_(NULL), outport_(NULL) {}

PortConnection::PortConnection(Outport* outport, Inport* inport)
    : inport_(inport), outport_(outport) {}

PortConnection::~PortConnection() {}

void PortConnection::serialize(IvwSerializer& s) const {
    s.serialize("OutPort", outport_);
    s.serialize("InPort", inport_);
}

void PortConnection::deserialize(IvwDeserializer& d) {
    bool out_error(false);
    std::string out_key;
    std::string out_type;

    bool in_error(false);
    std::string in_key;
    std::string in_type;

    Outport outport("");
    try {
        d.deserialize("OutPort", outport_);
    } catch (SerializationException& e) {
        out_error = true;
        out_key = e.getType();
        out_type = e.getKey();
    }

    Inport inport("");
    try {
        d.deserialize("InPort", inport_);
    } catch (SerializationException& e) {
        in_error = true;
        in_key = e.getType();
        in_type = e.getKey();
    }

    if (!(out_error || in_error)) {
        Processor* outPortProcessor = outport.getProcessor();
        if (outPortProcessor) {
            outport_ = outPortProcessor->getOutport(outport.getIdentifier());
        }
        Processor* inPortProcessor = inport.getProcessor();
        if (inPortProcessor) {
            inport_ = inPortProcessor->getInport(inport.getIdentifier());
        }

        if (!(outport_ && inport_)) {
            std::string type =
                (outPortProcessor ? outPortProcessor->getIdentifier() : "<Missing>") + "." +
                outport.getIdentifier() + " to " +
                (inPortProcessor ? inPortProcessor->getIdentifier() : "<Missing>") + "." +
                inport.getIdentifier();

            throw SerializationException("Could not create PortConnection: " + type,
                                         "PortConnection", type);
        }

    } else {
        std::string type = (out_error ? out_type : outport.getProcessor()->getIdentifier()) + "." +
                           outport.getIdentifier() + " to " +
                           (in_error ? in_type : inport.getProcessor()->getIdentifier()) + "." +
                           inport.getIdentifier();

        throw SerializationException("Could not create PortConnection: " + type, "PortConnection",
                                     type);
    }
}

}  // namespace
