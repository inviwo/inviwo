/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2023 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>

#include <inviwo/core/util/threadpool.h>

#include <string>
#include <thread>
#include <future>
#include <functional>

namespace inviwo {

class InviwoApplication;

namespace util {

IVW_CORE_API void setThreadDescription(const std::string& desc);

IVW_CORE_API int getPid();

IVW_CORE_API ThreadPool& getThreadPool();
IVW_CORE_API ThreadPool& getThreadPool(InviwoApplication* app);
IVW_CORE_API void waitForPool();
IVW_CORE_API void waitForPool(InviwoApplication* app);
IVW_CORE_API size_t processFront();
IVW_CORE_API size_t processFront(InviwoApplication* app);

/**
 * Utility function to query the pool size of the InviwoApplication
 * @return pool size of the InviwoApplication, 0 if the application is not initialized
 * @see InviwoApplication::getPoolSize(), InviwoApplication::isInitialized
 */
IVW_CORE_API size_t getPoolSize();

template <class F, class... Args>
auto dispatchPool(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
    return getThreadPool().enqueue(std::forward<F>(f), std::forward<Args>(args)...);
}

template <class F, class... Args>
auto dispatchPool(InviwoApplication* app, F&& f, Args&&... args)
    -> std::future<std::invoke_result_t<F, Args...>> {
    return getThreadPool(app).enqueue(std::forward<F>(f), std::forward<Args>(args)...);
}

IVW_CORE_API void dispatchFrontAndForget(std::function<void()> fun);
IVW_CORE_API void dispatchFrontAndForget(InviwoApplication* app, std::function<void()> fun);

template <class F, class... Args>
auto dispatchFront(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
    using return_type = std::invoke_result_t<F, Args...>;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();

    dispatchFrontAndForget(task);

    return res;
}

template <class F, class... Args>
auto dispatchFront(InviwoApplication* app, F&& f, Args&&... args)
    -> std::future<std::invoke_result_t<F, Args...>> {
    using return_type = std::invoke_result_t<F, Args...>;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();

    dispatchFrontAndForget(app, task);

    return res;
}

}  // namespace util
}  // namespace inviwo
