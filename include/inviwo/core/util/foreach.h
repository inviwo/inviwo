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
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <string>
#include <utility>

namespace inviwo {

namespace util {
    
namespace detail {
template <typename Callback, typename IT>
void foreach_helper(std::false_type, IT a, IT b, Callback&& callback, size_t) {
    std::for_each(a, b, std::forward<Callback>(callback));
}

template <typename Callback, typename IT>
void foreach_helper(std::true_type, IT a, IT b, Callback&& callback, size_t startIndex) {
    std::for_each(a, b, [&](auto&& v) { callback(v, startIndex++); });
}

template <typename Callback, typename IT, typename OnDoneCallback>
auto foreach_helper_pool(std::true_type, IT a, IT b, Callback&& callback, size_t startIndex,
                         OnDoneCallback&& onTaskDone) {
    return dispatchPool([id = startIndex, a, b, c = std::forward<Callback>(callback),
                         onTaskDone = std::forward<OnDoneCallback>(onTaskDone)]() mutable {
        std::for_each(a, b, [&](auto&& v) { c(v, id++); });
        onTaskDone();
    });
}

template <typename Callback, typename IT, typename OnDoneCallback>
auto foreach_helper_pool(std::false_type, IT a, IT b, Callback&& callback, size_t,
                         OnDoneCallback&& onTaskDone) {
    return dispatchPool([a, b, c = std::forward<Callback>(callback), onTaskDone = std::forward<OnDoneCallback>(onTaskDone)]() {
        std::for_each(a, b, c);
        onTaskDone();
    });
}

}  // namespace detail


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

    using std::begin;
    using std::end;
    
    auto settings = InviwoApplication::getPtr()->getSettingsByType<SystemSettings>();
    auto poolSize = settings->poolSize_.get();
    
    using value_type = decltype(*begin(iterable));
    using IncludeIndexType = typename util::is_invocable<Callback, value_type, size_t>::type;

    if (poolSize == 0) {
        detail::foreach_helper(IncludeIndexType{}, begin(iterable), end(iterable),
                               std::forward<Callback>(callback), 0);
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
        auto a = begin(iterable) + start;
        auto b = begin(iterable) + end;
        futures.push_back(
            detail::foreach_helper_pool(IncludeIndexType{}, a, b, std::forward<Callback>(callback),
                                        start, std::forward<OnDoneCallback>(onTaskDone)));
    }
    return futures;
}

template <typename Iterable, typename Callback>
std::vector<std::future<void>> forEachParallelAsync(const Iterable& iterable, Callback&& callback,
                                                    size_t jobs) {
    return forEachParallelAsync(iterable, std::forward<Callback>(callback), jobs, [](){});
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
