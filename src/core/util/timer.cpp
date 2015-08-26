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

Timer::Timer(duration_t interval, std::function<void()> callback)
    : callback_{std::make_shared<Callback>(std::move(callback))}
    , interval_{interval} {}

Timer::Timer(size_t interval, std::function<void()> callback)
    : Timer(std::chrono::milliseconds(interval), std::move(callback)) {}

Timer::~Timer() { stop(); }


void Timer::start(size_t interval) {
    start(std::chrono::milliseconds(interval));
}

void Timer::start(duration_t interval) {
    interval_ = interval; 
    start();
}

void Timer::start() {
    if (!callback_->enabled) {
        callback_->enabled = true;
        thread_ = std::thread(&Timer::timer, this);
    }
}

void Timer::setInterval(size_t interval) {
    setInterval(std::chrono::milliseconds(interval));
}

void Timer::setInterval(duration_t interval) {
    std::lock_guard<std::mutex> lock{callback_->mutex};
    interval_ = interval;
}

Timer::duration_t Timer::getInterval() const {
    std::lock_guard<std::mutex> lock{callback_->mutex};
    return interval_;
}

void Timer::stop() {
    if (callback_->enabled) {
        {
            std::unique_lock<std::mutex> l(callback_->mutex);
            callback_->enabled = false;
        }
        callback_->cvar.notify_one();
        thread_.join();
    }
}

void Timer::timer() {
    auto deadline = std::chrono::steady_clock::now() + interval_;
    std::unique_lock<std::mutex> lock{callback_->mutex};
    
    while (callback_->enabled) {
        if (callback_->cvar.wait_until(lock, deadline) == std::cv_status::timeout) {
            lock.unlock();
            // skip if previous not done.
            if (!result_.valid() ||
                result_.wait_for(std::chrono::duration<int, std::milli>(0)) ==
                    std::future_status::ready) {
                auto tmp = callback_;
                result_ = dispatchFront([tmp]() { 
                    std::unique_lock<std::mutex> l(tmp->mutex, std::try_to_lock);
                    if (l && tmp->enabled) (tmp->callback)();                     
                }
                );
            }
            deadline += interval_;
            lock.lock();
        }
    }
}

Delay::Delay(duration_t interval, std::function<void()> callback)
    : callback_{std::make_shared<std::function<void()>>(std::move(callback))}
    , interval_{interval}
    , enabled_{false} {}

Delay::Delay(size_t interval, std::function<void()> callback)
    : Delay(std::chrono::milliseconds(interval), std::move(callback)) {}

Delay::~Delay() { stop(); }

void Delay::start() {
    if (!enabled_) {
        enabled_ = true;
        thread_ = std::thread(&Delay::delay, this);
    }
}

void Delay::stop() {
    if (enabled_) {
        {
            std::lock_guard<std::mutex> _{mutex_};
            enabled_ = false;
        }
        cvar_.notify_one();
        thread_.join();
    }
}

void Delay::delay() {
    auto deadline = std::chrono::steady_clock::now() + interval_;
    std::unique_lock<std::mutex> lock{mutex_};
    while (enabled_) {
        if (cvar_.wait_until(lock, deadline) == std::cv_status::timeout) {
            lock.unlock();
            auto tmp = callback_;
            dispatchFront([tmp]() { (*tmp)(); });

            lock.lock();
            enabled_ = false;
        }
    }
}

}  // namespace inviwo
