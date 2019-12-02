/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/all>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <functional>
#include <memory>
#include <thread>
#include <future>
#include <warn/pop>

namespace inviwo {

class Timer;
class Delay;
/**
 * A background thread to be shared by many timers and delays.
 * Hold a queue of TimerInfo object with a time and a callback that will be called using
 * dispatchFront when the time is reached.
 * InviwoApplicaition owns a default TimerThread.
 */
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
        ControlBlock(std::function<void()> callback, Milliseconds interval);
        std::function<void()> callback_;
        Milliseconds interval_;
        std::future<void> finished_;
    };

    struct TimerInfo {
        TimerInfo(clock_t::time_point tp, std::weak_ptr<ControlBlock> controlBlock);

        TimerInfo(const TimerInfo &) = default;
        TimerInfo &operator=(const TimerInfo &) = default;
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
/**
 *	Utility function to get the default TimerThread from the app.
 */
IVW_CORE_API TimerThread &getDefaultTimerThread();
}  // namespace util

/**
 * A Timer class. Will evaluate it's callback in the front thread, with the period of interval
 */
class IVW_CORE_API Timer {
public:
    using Milliseconds = std::chrono::milliseconds;

    Timer(Milliseconds interval, std::function<void()> callback,
          TimerThread &thread = util::getDefaultTimerThread());
    ~Timer();

    void start(Milliseconds interval, std::function<void()> callback);
    void start(std::function<void()> callback);
    void start(Milliseconds interval);
    void start();
    void stop();

    void setInterval(Milliseconds interval);
    Milliseconds getInterval() const;

    void setCallback(std::function<void()> callback);
    std::function<void()> getCallback() const;

    bool isRunning() const;

private:
    std::function<void()> callback_;
    std::shared_ptr<TimerThread::ControlBlock> controlblock_;
    Milliseconds interval_{0};
    TimerThread &thread_;
};

/**
 *	A one time delay.
 */
class IVW_CORE_API Delay {
public:
    using Milliseconds = std::chrono::milliseconds;
    Delay(Milliseconds defaultDelay, std::function<void()> defaltCallback,
          TimerThread &thread = util::getDefaultTimerThread());
    ~Delay();

    void start(Milliseconds delay, std::function<void()> callback);
    void start(Milliseconds delay);
    void start(std::function<void()> callback);
    void start();
    void cancel();

    void setDefaultDelay(Milliseconds delay);
    Milliseconds getDefaultDelay() const;

    void setDefaultCallback(std::function<void()> callback);
    std::function<void()> getDefaultCallback() const;

private:
    std::function<void()> defaultCallback_;
    std::shared_ptr<TimerThread::ControlBlock> controlblock_;
    Milliseconds defaultDelay_;
    TimerThread &thread_;
};

}  // namespace inviwo

#endif  // IVW_TIMER_H
