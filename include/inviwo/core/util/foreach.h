/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#ifndef IVW_FOREACH_H
#define IVW_FOREACH_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/settings/systemsettings.h>

#include <utility>

namespace inviwo {

namespace util {

namespace detail {

template <typename Callback, typename IT>
void foreach (IT a, IT b, Callback && callback, size_t startIndex = 0) {
    using value_type = decltype(*a);
    if constexpr (std::is_invocable_v<Callback, value_type, size_t>) {
        std::for_each(a, b, [&](auto&& v) { callback(v, startIndex++); });
    } else {
        std::for_each(a, b, std::forward<Callback>(callback));
    }
}

}  // namespace detail

/**
 * Utility function to iterate over all element in an iterable data structure (such as std::vector).
 * @param iterable the data structure to iterate over
 * @param callback to call for each element, can be either `[](auto &a){}` or `[](auto &a,
 * size_t id){}` where `a` is an data item from the iterable data structure and `id` is the index in
 * the data structure
 */
template <typename Iterable, typename Callback>
void forEach(const Iterable& iterable, Callback&& callback) {
    detail::foreach (std::begin(iterable), std::end(iterable), std::forward<Callback>(callback));
}

/**
 * Use multiple threads to iterate over all elements in an iterable data structure (such as
 * std::vector). If the Inviwo pool size is zero it will be executed directly in the same thread as
 * the caller.
 * The function will return once all jobs as has been created and queued.
 *
 * @param iterable the data structure to iterate over
 * @param callback to call for each element, can be either `[](auto &a){}` or `[](auto &a,
 * size_t id){}` where `a` is an data item from the iterable data structure and `id` is the index in
 * the data structure
 * @param jobs optional parameter specifying how many jobs to create, if jobs==0 (default) it will
 * create pool size * 4 jobs
 * @param onTaskDone callback that will be called when each job is done
 * @return a vector of futures, one for each job created.
 */
template <typename Iterable, typename Callback, typename OnDoneCallback>
std::vector<std::future<void>> forEachParallelAsync(const Iterable& iterable, Callback&& callback,
                                                    size_t jobs, OnDoneCallback&& onTaskDone) {
    auto settings = InviwoApplication::getPtr()->getSettingsByType<SystemSettings>();
    const auto poolSize = settings->poolSize_.get();

    if (poolSize == 0) {
        forEach(iterable, std::forward<Callback>(callback));
        onTaskDone();
        return {};
    }

    if (jobs == 0) {  // If jobs is zero, set to 4 times the pool size
        jobs = 4 * poolSize;
    }

    const auto s = iterable.size();
    std::vector<std::future<void>> futures;
    for (size_t job = 0; job < jobs; ++job) {
        size_t start = (s * job) / jobs;
        size_t end = (s * (job + 1)) / jobs;
        auto a = std::begin(iterable) + start;
        auto b = std::begin(iterable) + end;
        auto future = dispatchPool([a, b, start, c = std::forward<Callback>(callback),
                                    onTaskDone = std::forward<OnDoneCallback>(onTaskDone)]() {
            detail::foreach (a, b, c, start);
            onTaskDone();
        });
        futures.push_back(std::move(future));
    }
    return futures;
}

template <typename Iterable, typename Callback>
std::vector<std::future<void>> forEachParallelAsync(const Iterable& iterable, Callback&& callback,
                                                    size_t jobs = 0) {
    return forEachParallelAsync(iterable, std::forward<Callback>(callback), jobs, []() {});
}

/**
 * Use multiple threads to iterate over all elements in an iterable data structure (such as
 * std::vector). If the Inviwo pool size is zero it will be executed directly in the same thread as
 * the caller.
 * The function will return once all jobs as has finished processing.
 *
 * @param iterable the data structure to iterate over
 * @param callback to call for each element, can be either `[](auto &a){}` or `[](auto &a,
 * size_t id){}` where `a` is an data item from the iterable data structure and `id` is the index in
 * the data structure
 * @param jobs optional parameter specifying how many jobs to create, if jobs==0 (default) it will
 * create pool size * 4 jobs
 */
template <typename Iterable, typename Callback>
void forEachParallel(const Iterable& iterable, Callback&& callback, size_t jobs = 0) {
    const auto futures =
        forEachParallelAsync<Iterable, Callback>(iterable, std::forward<Callback>(callback), jobs);

    for (const auto& e : futures) {
        e.wait();
    }
}

}  // namespace util

}  // namespace inviwo

#endif  // IVW_UTILITIES_H
