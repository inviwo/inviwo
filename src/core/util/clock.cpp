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

#include <inviwo/core/util/clock.h>
#include <inviwo/core/util/logcentral.h>

namespace inviwo {

Clock::Clock() {
}

void Clock::start() {
    startTime_ = std::chrono::high_resolution_clock::now();
    tickTime_ = startTime_;
}

void Clock::tick() {
    tickTime_ = std::chrono::high_resolution_clock::now();
}

float Clock::getElapsedMiliseconds() const { return 1000.f * getElapsedSeconds(); }

float Clock::getElapsedSeconds() const {
    using std::chrono::duration_cast;
    using std::chrono::duration;
    return (duration_cast<duration<float>>(tickTime_ - startTime_)).count();
}

void ScopedClockCPU::print() {
    clock_.tick();
    if (clock_.getElapsedMiliseconds() > logIfAtLeastMilliSec_) {
        std::stringstream message;
        message << logMessage_ << ": " << clock_.getElapsedMiliseconds() << " ms";
        LogCentral::getPtr()->log(logSource_, LogLevel::Info, LogAudience::Developer, __FILE__,
                                  __FUNCTION__, __LINE__, message.str());
    }
}

void ScopedClockCPU::reset() {
    clock_.start();
}

void ScopedClockCPU::printAndReset() {
    print();
    reset();
}

ScopedClockCPU::~ScopedClockCPU() {
    print();
}

}  // namespace