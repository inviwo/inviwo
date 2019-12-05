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

#include <inviwo/core/util/timer.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/stdextensions.h>

#include <warn/push>
#include <warn/ignore/all>
#include <algorithm>
#include <utility>
#include <warn/pop>

namespace inviwo {

TimerThread::TimerThread()
    : sort_(false)
    , stop_(false)
    , thread_{std::make_unique<std::thread>([this]() { TimerLoop(); })} {}

TimerThread::~TimerThread() {
    stop_ = true;
    condition_.notify_all();
    thread_->join();
}

void TimerThread::add(std::weak_ptr<ControlBlock> controlBlock) {
    if (auto cb = controlBlock.lock()) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            timers_.emplace_back(clock_t::now() + cb->interval_, std::move(controlBlock));
            sort_ = true;
        }
        // wake up
        condition_.notify_one();
    }
}

void TimerThread::TimerLoop() {
    for (;;) {
        std::unique_lock<std::mutex> lock(mutex_);

        // Sleep while queue is empty
        while (!stop_ && timers_.empty()) {
            condition_.wait(lock);
        }

        if (stop_) {
            return;
        }

        if (sort_) {
            // Sort could be done at insert
            // but probably this thread has time to do
            std::sort(timers_.begin(), timers_.end(),
                      [](const TimerInfo &ti1, const TimerInfo &ti2) {
                          return ti1.timePoint_ > ti2.timePoint_;
                      });
            sort_ = false;
        }

        auto now = clock_t::now();
        auto expire = timers_.back().timePoint_;
        bool callTimer = expire < now;

        if (!callTimer)  // can I take a nap?
        {
            auto napTime = expire - now;
            condition_.wait_for(lock, napTime);

            if (!timers_.empty()) {
                // check again
                auto expire2 = timers_.back().timePoint_;
                auto now2 = clock_t::now();
                callTimer = expire2 < now2;
            }
        }

        if (callTimer) {
            if (auto cb = timers_.back().controlBlock_.lock()) {
                if (cb->interval_ > Milliseconds(0)) {
                    timers_.back().timePoint_ += cb->interval_;
                    sort_ = true;
                } else {
                    timers_.pop_back();
                }
                if (!cb->finished_.valid() ||
                    cb->finished_.wait_for(std::chrono::duration<int, std::milli>(0)) ==
                        std::future_status::ready) {
                    cb->finished_ = dispatchFront([ctrlblk = timers_.back().controlBlock_]() {
                        if (auto cb2 = ctrlblk.lock()) {
                            cb2->callback_();
                        }
                    });
                }
            } else {
                timers_.pop_back();
            }
        }
    }
}

TimerThread::ControlBlock::ControlBlock(std::function<void()> callback, Milliseconds interval)
    : callback_(std::move(callback)), interval_{interval} {}

TimerThread::TimerInfo::TimerInfo(clock_t::time_point tp, std::weak_ptr<ControlBlock> controlBlock)
    : timePoint_(tp), controlBlock_(std::move(controlBlock)) {}

Timer::Timer(Milliseconds interval, std::function<void()> callback, TimerThread &thread)
    : callback_{std::move(callback)}, interval_{interval}, thread_{thread} {}

Timer::~Timer() { stop(); }

void Timer::start(Milliseconds interval, std::function<void()> callback) {
    interval_ = interval;
    callback_ = callback;
    controlblock_ = std::make_shared<TimerThread::ControlBlock>(callback, interval);
    thread_.add(controlblock_);
}
void Timer::start(Milliseconds interval) { start(interval, callback_); }

void Timer::start(std::function<void()> callback) { start(interval_, callback); }
void Timer::start() { start(interval_, callback_); }

void Timer::setInterval(Milliseconds interval) {
    if (controlblock_) {
        start(interval);
    } else {
        interval_ = interval;
    }
}

void Timer::setCallback(std::function<void()> callback) {
    callback_ = std::move(callback);
    if (controlblock_) {
        start();
    }
}

std::function<void()> Timer::getCallback() const { return callback_; }

Timer::Milliseconds Timer::getInterval() const {
    if (controlblock_) {
        return controlblock_->interval_;
    } else {
        return interval_;
    }
}

bool Timer::isRunning() const { return controlblock_ != nullptr; }

void Timer::stop() { controlblock_.reset(); }

Delay::Delay(Milliseconds defaultDelay, std::function<void()> callback, TimerThread &thread)
    : defaultCallback_{std::move(callback)}, defaultDelay_{defaultDelay}, thread_{thread} {}

Delay::~Delay() { cancel(); }

void Delay::start(Milliseconds delay, std::function<void()> callback) {
    controlblock_ = std::make_shared<TimerThread::ControlBlock>(callback, delay);
    thread_.add(controlblock_);
}
void Delay::start(Milliseconds delay) { start(delay, defaultCallback_); }
void Delay::start(std::function<void()> callback) { start(defaultDelay_, callback); }
void Delay::start() { start(defaultDelay_, defaultCallback_); }

void Delay::cancel() { controlblock_.reset(); }

void Delay::setDefaultDelay(Milliseconds delay) { defaultDelay_ = delay; }
auto Delay::getDefaultDelay() const -> Milliseconds { return defaultDelay_; }

void Delay::setDefaultCallback(std::function<void()> callback) { defaultCallback_ = callback; }
std::function<void()> Delay::getDefaultCallback() const { return defaultCallback_; }

namespace util {

TimerThread &getDefaultTimerThread() {
    if (auto app = InviwoApplication::getPtr()) {
        return app->getTimerThread();
    } else {
        throw Exception("No timer thread found", IVW_CONTEXT_CUSTOM("TimerThread"));
    }
}

}  // namespace util

}  // namespace inviwo
