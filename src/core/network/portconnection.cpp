/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

PortConnection::PortConnection() : inport_(nullptr), outport_(nullptr) {}

PortConnection::PortConnection(Outport* outport, Inport* inport)
    : inport_(inport), outport_(outport) {}

PortConnection::~PortConnection() {}

void PortConnection::serialize(Serializer& s) const {
    s.serialize("OutPort", outport_);
    s.serialize("InPort", inport_);
}

void PortConnection::deserialize(Deserializer& d) {
    struct SError {
        SError() : error(false), data(){};
        bool error;
        SerializationException::SerializationExceptionData data;
    };
    SError in, out;

    try {
        d.deserialize("OutPort", outport_);
    } catch (SerializationException& e) {
        out.error = true;
        out.data = e.getData();
    }

    try {
        d.deserialize("InPort", inport_);
    } catch (SerializationException& e) {
        in.error = true;
        in.data = e.getData();
    }

    if (out.error && in.error) {
        throw SerializationException("Could not create Connection from \"" +
                                     out.data.nd.getDescription() + " to " +
                                     in.data.nd.getDescription() +
                                     ". Outport and Inport not found.", IvwContext,
                                     "Connection");
    } else if (out.error) {
        std::string message = "Could not create Connection from \"" + out.data.nd.getDescription() +
                              " to port \"" + inport_->getIdentifier() + "\" in processor \"" +
                              inport_->getProcessor()->getIdentifier() + "\". Outport not found";
        delete outport_;
        throw SerializationException(message, IvwContext, "Connection");
    } else if (in.error) {
        std::string message = "Could not create Connection from port \"" +
                              outport_->getIdentifier() + "\" in processor \"" +
                              outport_->getProcessor()->getIdentifier() + "\" to " +
                              in.data.nd.getDescription() + ". Inport not found";
        delete inport_;
        throw SerializationException(message, IvwContext, "Connection");
    }
}

}  // namespace
