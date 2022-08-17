/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2022 Inviwo Foundation
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
#include <fmt/format.h>

#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/processors/processor.h>

namespace inviwo {

PortConnection::PortConnection() : inport_(nullptr), outport_(nullptr) {}

PortConnection::PortConnection(Outport* outport, Inport* inport)
    : inport_(inport), outport_(outport) {}

PortConnection::operator bool() const { return inport_ && outport_; }

bool PortConnection::involvesProcessor(Processor* processor) const {
    return (inport_->getProcessor() == processor || outport_->getProcessor() == processor);
}

bool operator==(const PortConnection& lhs, const PortConnection& rhs) {
    return lhs.outport_ == rhs.outport_ && lhs.inport_ == rhs.inport_;
}
bool operator!=(const PortConnection& lhs, const PortConnection& rhs) {
    return !operator==(lhs, rhs);
}

bool operator<(const PortConnection& lhs, const PortConnection& rhs) {
    if (lhs.outport_ != rhs.outport_) {
        return lhs.outport_ < rhs.outport_;
    } else {
        return lhs.inport_ < rhs.inport_;
    }
}

}  // namespace inviwo
