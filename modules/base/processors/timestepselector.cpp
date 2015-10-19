/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include "timestepselector.h"

namespace inviwo {

ProcessorClassIdentifier(TimeStepSelector, "org.inviwo.TimeStepSelector");
ProcessorDisplayName(TimeStepSelector, "Time Step Selector");
ProcessorTags(TimeStepSelector, Tags::CPU);
ProcessorCategory(TimeStepSelector, "Volume Operation");
ProcessorCodeState(TimeStepSelector, CODE_STATE_STABLE);

TimeStepSelector::TimeStepSelector()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , timeStep_("timeStep", "Step") {
    addPort(inport_);
    addPort(outport_);

    addProperty(timeStep_);

    inport_.onChange([this]() {
        if(inport_.hasData()) timeStep_.updateMax(inport_.getData()->size());
    });
}

TimeStepSelector::~TimeStepSelector() {}

void TimeStepSelector::process() {
    if (!inport_.isReady()) return;

    if (auto data = inport_.getData()) {
        size_t index = std::min(data->size() - 1, static_cast<size_t>(timeStep_.index_.get() - 1));

        outport_.setData((*data)[index]);
    }
}

}  // namespace
