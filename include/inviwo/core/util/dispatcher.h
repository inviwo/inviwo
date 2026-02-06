/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2026 Inviwo Foundation
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

#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <algorithm>
#include <mutex>

namespace inviwo {

/**
 * Dispatches functions to a number of callbacks and cleans up callbacks when
 * they are dead. Thread-safe via copy-on-write semantics on the callback vector.
 *
 * - add() is implemented with a CAS loop that copies the callback vector,
 *   appends the new callback and attempts to swap it in.
 * - invoke() performs a single load of the callback vector and
 *   invokes all live callbacks. No locks are held during invocation.
 *
 */
template <typename C>
class Dispatcher final {
public:
    Dispatcher() noexcept = default;

    // Copying will reset the callbacks
    Dispatcher(const Dispatcher&) : callbacks_{} {}
    Dispatcher& operator=(const Dispatcher& that) {
        if (this != &that) {
            const std::scoped_lock lock{mutex_};
            callbacks_.reset();
        }
        return *this;
    }

    // Allow move semantics
    Dispatcher(Dispatcher&&) noexcept = default;
    Dispatcher& operator=(Dispatcher&&) noexcept = default;
    ~Dispatcher() = default;

    using Function = C;
    using Callback = std::function<C>;
    using Handle = std::shared_ptr<Callback>;

    template <typename T>
    Handle add(T&& callback) {
        auto shared = std::make_shared<Callback>(std::forward<T>(callback));
        auto newVec = std::make_shared<std::vector<std::weak_ptr<Callback>>>();
        // CAS loop: load current vector, copy-and-modify, try to swap
        for (;;) {
            std::shared_ptr<const std::vector<std::weak_ptr<Callback>>> currentVec;
            newVec->clear();
            {
                const std::scoped_lock lock{mutex_};
                currentVec = callbacks_;
            }
            if (currentVec) {
                newVec->reserve(currentVec->size());
                std::ranges::copy_if(
                    *currentVec, std::back_inserter(*newVec),
                    [](const std::weak_ptr<Callback>& wcb) { return !wcb.expired(); });
            }
            newVec->emplace_back(shared);

            {
                const std::scoped_lock lock{mutex_};
                if (currentVec == callbacks_) {  // nothing changed we can swap in the new one
                    callbacks_ = newVec;
                    return shared;
                }
            }
        }
    }

    template <typename... A>
    void invoke(A&&... args) {
        std::shared_ptr<const std::vector<std::weak_ptr<Callback>>> currentVec;
        {
            const std::scoped_lock lock{mutex_};
            currentVec = callbacks_;
        }
        if (!currentVec) return;

        bool hasExpired = false;
        for (auto& wcb : *currentVec) {
            if (auto cb = wcb.lock()) {
                std::invoke(*cb, std::forward<A>(args)...);
            } else {
                hasExpired = true;
            }
        }

        if (hasExpired) {
            auto newVec = std::make_shared<std::vector<std::weak_ptr<Callback>>>();
            newVec->reserve(currentVec->size());
            std::ranges::copy_if(*currentVec, std::back_inserter(*newVec),
                                 [](const std::weak_ptr<Callback>& wcb) { return !wcb.expired(); });

            // We only try once here.
            std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
            if (lock.try_lock() && currentVec == callbacks_) {
                callbacks_ = newVec;
            }
        }
    }

private:
    // Note std::atomic<std::shared_ptr<>> is not supported as of clang 21
    std::mutex mutex_;
    std::shared_ptr<const std::vector<std::weak_ptr<Callback>>> callbacks_;
};

template <typename C>
using DispatcherHandle = typename Dispatcher<C>::Handle;

}  // namespace inviwo
