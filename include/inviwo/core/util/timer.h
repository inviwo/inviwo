/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2017 Inviwo Foundation
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

// based on ideas from http://thradams.com/timers.htm

#ifndef IVW_TIMER_H
#define IVW_TIMER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwoapplication.h>

#include <iostream>
#include <warn/push>
#include <warn/ignore/all>
#include <chrono>
#include <condition_variable>
#include <thread>
#include <future>
#include <mutex>
#include <memory>
#include <utility>
#include <atomic>
#include <algorithm>
#include <warn/pop>

namespace inviwo {

class Timer;
class Delay;

class IVW_CORE_API TimerThread {
public:
    using Milliseconds = std::chrono::milliseconds;
    using clock_t = std::chrono::high_resolution_clock;
    
    TimerThread();
    ~TimerThread();

private:
    friend Timer;
    friend Delay;
    struct ControlBlock {
        ControlBlock(std::function<void()> callback, Milliseconds interval)
            : callback_(std::move(callback)), interval_{interval} {};
        std::function<void()> callback_;
        Milliseconds interval_;
    };

    struct TimerInfo {
        TimerInfo(clock_t::time_point tp, std::weak_ptr<ControlBlock> controlBlock)
            : timePoint_(tp), controlBlock_(std::move(controlBlock)) {}

        TimerInfo(const TimerInfo&) = default;
        TimerInfo& operator=(const TimerInfo&) = default;
        TimerInfo(TimerInfo &&) = default;
        TimerInfo &operator=(TimerInfo &&) = default;

        clock_t::time_point timePoint_;
        std::weak_ptr<ControlBlock> controlBlock_;
    };

    void add(std::weak_ptr<ControlBlock> controlBlock);
    void TimerLoop();
    

    std::vector<TimerInfo> timers_;
    std::mutex mutex_;
    std::condition_variable condition_;
    bool sort_;
    bool stop_;
    std::unique_ptr<std::thread> thread_;
};

namespace util {
TimerThread &getDefaultTimerThread();
}

/** \class Timer
 *
 * A Timer class. Will evaluate it's callback in the front thread.
 */
class IVW_CORE_API Timer {
public:
    using Milliseconds = std::chrono::milliseconds;

    Timer(Milliseconds interval, std::function<void()> callback,
          TimerThread &thread = util::getDefaultTimerThread());
    ~Timer();

    void start(Milliseconds interval);
    void setInterval(Milliseconds interval);
    void setCallback(std::function<void()> callback);
    void start();
    Milliseconds getInterval() const;
    bool isRunning() const;
    void stop();

private:
    static TimerThread &getDefault();

    std::function<void()> callback_;
    std::shared_ptr<TimerThread::ControlBlock> controlblock_;
    Milliseconds interval_{0};
    TimerThread &thread_;
};

class IVW_CORE_API Delay {
public:
    using Milliseconds = std::chrono::milliseconds;
    Delay(Milliseconds delay, std::function<void()> callback,
          TimerThread &thread = util::getDefaultTimerThread());
    ~Delay();

    void start();
    void cancel();

private:
    static TimerThread &getDefault();

    std::function<void()> callback_;
    std::shared_ptr<TimerThread::ControlBlock> controlblock_;
    Milliseconds interval_{0};
    TimerThread &thread_;
};

}  // namespace inviwo

#endif // IVW_TIMER_H
