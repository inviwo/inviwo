/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <modules/example/exampleprogressbar.h>
#include <thread>

namespace inviwo {

const ProcessorInfo ExampleProgressBar::processorInfo_{
    "org.inviwo.ExampleProgressBar",  // Class identifier
    "Example Progress Bar",           // Display name
    "Various",                        // Category
    CodeState::Stable,                // Code state
    Tags::CPU,                        // Tags
};
const ProcessorInfo ExampleProgressBar::getProcessorInfo() const { return processorInfo_; }

ExampleProgressBar::ExampleProgressBar()
    : PoolProcessor(pool::Option::DelayDispatch)
    , inport_("inputImage")
    , outport_("outputImage")
    , delay_{"delay", "Delay (ms)", 10, 0, 100, 1} {

    addPort(inport_);
    addPort(outport_);

    addProperties(delay_);
}

ExampleProgressBar::~ExampleProgressBar() = default;

void ExampleProgressBar::process() {

    // Construct a calculation. The calculation should capture all its state by value since it will
    // be evaluated on a different thread and any references might be dangling at the time of
    // execution. It should be noted that this is also true for the processor itself, which might
    // have been deleted before the calculation starts hence one can never capture `this` in the
    // lambda. The calculation can take to optional arguments `pool::Stop` which can be queried to
    // see if the calculation has been canceled and `pool::Progress` which can be called with a
    // float [0,1] to report the progress of the calculation. If `pool::Progress` is present then
    // the processor will show a ProgressBar in the NetworkEditor representing the progress
    const auto calc = [delay = delay_.get(), image = inport_.getData()](
                          pool::Stop stop,
                          pool::Progress progress) -> std::shared_ptr<const Image> {
        const int numSteps = 100;
        for (int i = 0; i < 100; ++i) {
            if (stop) return nullptr;
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            progress(i, numSteps);
        }
        return image;
    };

    outport_.clear();  // Discard any data already set since it is no longer valid

    // Dispatch the calculation to the thread pool. The second argument is a callback
    // that will get evaluated on the main thread after the calculation is done. The callback should
    // take the result of the calculation as argument.
    dispatchOne(calc, [this](std::shared_ptr<const Image> result) {
        outport_.setData(result);
        newResults();  // Let the network know that the processor has new results on the outport.
    });
}

}  // namespace inviwo
