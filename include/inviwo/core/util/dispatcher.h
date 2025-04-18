/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

#include <inviwo/core/util/raiiutils.h>

#include <vector>
#include <memory>
#include <functional>
#include <algorithm>

namespace inviwo {

// Based on:
// http://nercury.github.io/c++/interesting/2016/02/22/weak_ptr-and-event-cleanup.html

/**
 * Dispatches function on a number of callbacks and cleans up callbacks when
 * they are dead.
 */
template <typename C>
class Dispatcher final {
public:
    using Function = C;
    using Callback = std::function<C>;
    using Handle = std::shared_ptr<std::function<C>>;

    template <typename T>
    std::shared_ptr<std::function<C>> add(T&& callback) {
        removeExpiredCallbacks();

        auto shared = std::make_shared<std::function<C>>(std::forward<T>(callback));
        callbacks.push_back(shared);
        return shared;
    }

    template <typename... A>
    void invoke(A&&... args) {
        ++concurrent_dispatcher_count;
        util::OnScopeExit decreaseCount{[this]() {
            --concurrent_dispatcher_count;
            removeExpiredCallbacks();
        }};

        // Go over all callbacks and dispatch on those that are still available.
        // don't use iterators here, they might be invalidated.
        const size_t size = callbacks.size();
        for (size_t i = 0; i < size; ++i) {
            if (auto callback = callbacks.at(i).lock()) {
                std::invoke(*callback, std::forward<A>(args)...);
            }
        }
    }

private:
    void removeExpiredCallbacks() {
        // Remove all callbacks that are gone, only if we are not dispatching.
        if (concurrent_dispatcher_count == 0) {
            std::erase_if(callbacks, [](const std::weak_ptr<std::function<C>>& callback) {
                return callback.expired();
            });
        }
    }

    std::vector<std::weak_ptr<std::function<C>>> callbacks;
    int32_t concurrent_dispatcher_count = 0;
};

template <typename C>
using DispatcherHandle = typename Dispatcher<C>::Handle;

}  // namespace inviwo
