/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

#include <inviwo/core/network/networklock.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/processors/processor.h>

namespace inviwo {

NetworkLock::NetworkLock(NetworkLock&& rhs) noexcept : network_(rhs.network_) {
    rhs.network_ = nullptr;
}
NetworkLock& NetworkLock::operator=(NetworkLock&& that) {
    NetworkLock lock(std::move(that));
    std::swap(network_, lock.network_);
    return *this;
}

NetworkLock::NetworkLock() : network_(InviwoApplication::getPtr()->getProcessorNetwork()) {
    if (network_) network_->lock();
}

NetworkLock::NetworkLock(ProcessorNetwork* network) : network_(network) {
    if (network_) network_->lock();
}

NetworkLock::NetworkLock(Processor* processor)
    : NetworkLock(processor ? processor->getNetwork() : nullptr) {}

NetworkLock::NetworkLock(Property* property)
    : NetworkLock(property ? (property->getOwner() ? property->getOwner()->getProcessor() : nullptr)
                           : nullptr) {}

NetworkLock::~NetworkLock() {
    if (network_) network_->unlock();
}

}  // namespace inviwo
