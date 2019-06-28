/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#ifndef IVW_NETWORKLOCK_H
#define IVW_NETWORKLOCK_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/processors/processor.h>

namespace inviwo {

// A RAII utility for locking and unlocking the network
struct IVW_CORE_API NetworkLock {
    NetworkLock();
    NetworkLock(ProcessorNetwork* network);
    NetworkLock(Processor* network);
    NetworkLock(Property* network);
    ~NetworkLock();

    NetworkLock(NetworkLock const&) = delete;
    NetworkLock& operator=(NetworkLock const& that) = delete;
    NetworkLock(NetworkLock&& rhs);
    NetworkLock& operator=(NetworkLock&& that);

private:
    ProcessorNetwork* network_;
};

inline NetworkLock::NetworkLock(ProcessorNetwork* network) : network_(network) {
    if (network_) network_->lock();
}

inline NetworkLock::NetworkLock(Processor* processor)
    : NetworkLock(processor ? processor->getNetwork() : nullptr) {}

inline NetworkLock::NetworkLock(Property* property)
    : NetworkLock(property ? (property->getOwner() ? property->getOwner()->getProcessor() : nullptr)
                           : nullptr) {}

inline NetworkLock::~NetworkLock() {
    if (network_) network_->unlock();
}

}  // namespace inviwo

#endif  // IVW_NETWORKLOCK_H
