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

#include "exampleprogressbar.h"
#include <thread>

namespace inviwo {

const ProcessorInfo ExampleProgressBar::processorInfo_{
    "org.inviwo.ExampleProgressBar",  // Class identifier
    "Example Progress Bar",           // Display name
    "Various",                        // Category
    CodeState::Experimental,          // Code state
    Tags::None,                       // Tags
};
const ProcessorInfo ExampleProgressBar::getProcessorInfo() const {
    return processorInfo_;
}

ExampleProgressBar::ExampleProgressBar()
    : Processor(), inport_("image.inport"), outport_("image.outport") {
    addPort(inport_);
    addPort(outport_);

    // initially hide progress bar
    // progressBar_.hide();
}

ExampleProgressBar::~ExampleProgressBar() {}

void ExampleProgressBar::process() {
    // progressBar_.show();

    // reset progress bar
    progressBar_.resetProgress();

    const int sleepTime = 10;

    const int numSteps = 100;
    for (int i = 0; i < numSteps; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
        // update progress bar state
        updateProgress(i / static_cast<float>(numSteps));
    }

    // set progress bar to 100%
    progressBar_.finishProgress();
    // progressBar_.hide();

    // dummy operation, pass on image data
    outport_.setData(inport_.getData());
}

}  // inviwo namespace

