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

#include <inviwo/core/util/timer.h>

namespace inviwo {

#ifdef WIN32

void CALLBACK TimerCallback(void* param, bool timerOrWaitFired) {
    WindowsTimer* timer = static_cast<WindowsTimer*>(param);
    timer->onIntervalEvent();
}

WindowsTimer::WindowsTimer() :Timer(), timer_(nullptr) {
}

WindowsTimer::~WindowsTimer() {
    stop();
}

void WindowsTimer::start(unsigned int intervalInMilliseconds, bool once /*= false*/) {
    if (timer_)
        return;

    CreateTimerQueueTimer(&timer_,
                          nullptr,
                          (WAITORTIMERCALLBACK)TimerCallback,
                          this,
                          0,
                          once ? 0 : intervalInMilliseconds,
                          WT_EXECUTEINTIMERTHREAD);
}

void WindowsTimer::stop() {
    DeleteTimerQueueTimer(nullptr, timer_, nullptr);
    timer_ = nullptr ;
}

#endif // WIN32

} // namespace inviwo

