/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

//static const double CLOCKS_PER_MS = CLOCKS_PER_SEC / 1000.0;
//static const double INV_CLOCKS_PER_MS = 1.0 / CLOCKS_PER_MS; Not used? /Peter


namespace inviwo {

Clock::Clock() {
#ifdef WIN32
    QueryPerformanceFrequency(&ticksPerSecond_);
#endif
}

void Clock::start() {
#ifdef WIN32
    QueryPerformanceCounter(&startTime_);
#else
    startTime_ = clock();
#endif
}

void Clock::stop() {
#ifdef WIN32
    QueryPerformanceCounter(&stopTime_);
#else
    stopTime_ = clock();
#endif
}

float Clock::getElapsedMiliseconds() const {
    return 1000.f*getElapsedSeconds();
}

float Clock::getElapsedSeconds() const {
#ifdef WIN32
    return static_cast<float>(stopTime_.QuadPart-startTime_.QuadPart) / ticksPerSecond_.QuadPart;
#else
    return static_cast<float>(stopTime_ - startTime_)/static_cast<float>(CLOCKS_PER_SEC);
#endif
}



ScopedClockCPU::~ScopedClockCPU() {
    clock_.stop();
    if (clock_.getElapsedMiliseconds() > logIfAtLeastMilliSec_) {
        std::stringstream message;
        message << logMessage_ << ": " << clock_.getElapsedMiliseconds() << " ms";
        LogCentral::getPtr()->log(logSource_, inviwo::Info, __FILE__, __FUNCTION__, __LINE__, message.str());
    }
}

}//namespace