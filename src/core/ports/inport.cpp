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
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

Inport::Inport(std::string identifier) : Port(identifier), changed_(false) {}

Inport::~Inport() {}

bool Inport::isReady() const {
    return isConnected() && getConnectedOutport()->getInvalidationLevel() == VALID;
}

void Inport::invalidate(InvalidationLevel invalidationLevel) {
    if (processor_) processor_->invalidate(invalidationLevel);
}

std::vector<Processor*> Inport::getPredecessors() const {
    std::vector<Processor*> predecessors;
    getPredecessors(predecessors);
    return predecessors;
}

void Inport::getPredecessors(std::vector<Processor*>& predecessors) const {
    for (auto outport : getConnectedOutports()) {
        Processor* p = outport->getProcessor();

        if (std::find(predecessors.begin(), predecessors.end(), p) == predecessors.end()) {
            predecessors.push_back(p);
            for (auto inport : p->getInports()) {
                inport->getPredecessors(predecessors);
            }
        }
    }
}

void Inport::setChanged(bool changed) { changed_ = changed; }

void Inport::removeOnChange(const BaseCallBack* callback) {
    onChangeCallback_.remove(callback);
}

void Inport::callOnChangeIfChanged() const {
    if (isChanged()) {
        onChangeCallback_.invokeAll();
    }
}

const BaseCallBack* Inport::onChange(std::function<void()> lambda) const {
    return onChangeCallback_.addLambdaCallback(lambda);
}

bool Inport::isChanged() const { return changed_; }

}  // namespace
