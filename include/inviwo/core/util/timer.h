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

#ifndef IVW_TIMER_H
#define IVW_TIMER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <iostream>
#include <warn/push>
#include <warn/ignore/all>
#include <chrono>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <memory>
#include <utility>
#include <warn/pop>

#ifdef WIN32
// For WindowsTimer
#include <windows.h>
#endif

namespace inviwo {

/** \class Timer
 *
 * Interface for Timer classes.
 * A class deriving from Timer should execute onIntervalEvent() when the interval has passed.
 */
class IVW_CORE_API Timer {
public:
    Timer() {};
    virtual ~Timer() { }

    /**
     * Start the timer.
     *
     * @param intervalInMilliseconds The time interval until the added callback will be called.
     * @param once Should the callback only be called the first time the interval has been reached?
     */
    virtual void start(unsigned int intervalInMilliseconds, bool once = false) = 0;

    /**
     * Stop the timer from calling the callback.
     *
     * @return
     */
    virtual void stop() = 0;

    /**
     * Set a callback that will be called when the time interval has passed.
     * Only one callback may be set.
     */
    template <typename T>
    void setElapsedTimeCallback(T* o, void (T::*m)()) {
        callback_ = std::bind(m, o);
    }
    /**
     * This function will be called when the time has elapsed.
     * Callbacks will then be executed.
     *
     * @note Derived classes should call this function when the time has elapsed.
     */
    void onIntervalEvent() const {
        callback_();
    }
protected:
    std::function<void()> callback_;
};

#ifdef WIN32
/**
 * Will be called when WindowsTimer reaches time interval.
 *
 * @param param Will be a pointer to WindowsTimer
 * @param timerOrWaitFired
 */
static void CALLBACK TimerCallback(void* param, bool timerOrWaitFired);

/** \class WindowsTimer
 *
 * Windows only timer. Uses the Windows API to create a timed event. Typically millisecond resolution.
 * @note Does not work when used with Qt together with OpenGL (probably due to OpenGL context and parallel execution)
 * @see Timer
 */
class IVW_CORE_API WindowsTimer: public Timer {
public:
    WindowsTimer();
    virtual ~WindowsTimer();

    virtual void start(unsigned int intervalInMilliseconds, bool once = false);
    virtual void stop();


protected:
    HANDLE timer_;
};

#endif // WIN32

class IVW_CORE_API IvwTimer {
public:
    using duration_t = std::chrono::milliseconds;

    IvwTimer(size_t interval, std::function<void()> fun) :
        IvwTimer(std::chrono::milliseconds(interval), fun) {}
    
    IvwTimer(duration_t interval, std::function<void()> fun)
        : fun_{std::move(fun)}, interval_{interval}, enabled_{false} {}

    ~IvwTimer() {
        stop();
    }
    void start() {
        if (!enabled_) {
            enabled_ = true;
            thread_ = std::thread(&IvwTimer::timer, this);
        }
    }
    void stop() {
        if (enabled_) {
            {
                std::lock_guard<std::mutex> _{mutex_};
                enabled_ = false;
            }
            cvar_.notify_one();
            thread_.join();
        }
    }

private:
    void timer() {
        auto deadline = std::chrono::steady_clock::now() + interval_;
        std::unique_lock<std::mutex> lock{mutex_};
        while (enabled_) {
            if (cvar_.wait_until(lock, deadline) == std::cv_status::timeout) {
                lock.unlock();
                fun_();
                deadline += interval_;
                lock.lock();
            }
        }
    }

    std::function<void()> fun_;
    const duration_t interval_;
    
    bool enabled_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cvar_;
};

}; // namespace inviwo

#endif // IVW_TIMER_H