/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <modules/brushingandlinking/events/brushingandlinkingevent.h>
#include <modules/brushingandlinking/events/filteringevent.h>
#include <modules/brushingandlinking/events/selectionevent.h>
#include <modules/brushingandlinking/processors/brushingandlinkingprocessor.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo BrushingAndLinkingProcessor::processorInfo_{
    "org.inviwo.BrushingAndLinkingProcessor",  // Class identifier
    "Brushing And Linking Processor",          // Display name
    "Brushing And Linking",                    // Category
    CodeState::Experimental,                   // Code state
    Tags::None,                                // Tags
};
const ProcessorInfo BrushingAndLinkingProcessor::getProcessorInfo() const { return processorInfo_; }

void BrushingAndLinkingProcessor::invokeEvent(Event* event) {
    if (auto brushingEvent = dynamic_cast<BrushingAndLinkingEvent*>(event)) {
        if (dynamic_cast<FilteringEvent*>(event)) {
            manager_->setFiltered(brushingEvent->getSource(), brushingEvent->getIndices());
            event->markAsUsed();
        } else if (dynamic_cast<SelectionEvent*>(event)) {
            manager_->setSelected(brushingEvent->getSource(), brushingEvent->getIndices());
            event->markAsUsed();
        } else if (dynamic_cast<ColumnSelectionEvent*>(event)) {
            manager_->setSelectedColumn(brushingEvent->getSource(), brushingEvent->getIndices());
            event->markAsUsed();
        }
    }
    Processor::invokeEvent(event);
}

BrushingAndLinkingProcessor::BrushingAndLinkingProcessor()
    : Processor()
    , outport_("outport")
    , manager_(std::make_shared<BrushingAndLinkingManager>(this)) {
    addPort(outport_);
}

void BrushingAndLinkingProcessor::process() { outport_.setData(manager_); }

}  // namespace inviwo
