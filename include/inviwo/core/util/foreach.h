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

namespace inviwo {

namespace util {
namespace detail {
template <typename Callback, typename IT>
void foreach_helper(std::false_type, IT a, IT b, Callback callback, size_t /*startIndex*/ = 0) {
    std::for_each(a, b, callback);
}

template <typename Callback, typename IT>
void foreach_helper(std::true_type, IT a, IT b, Callback callback, size_t startIndex = 0) {
    std::for_each(a, b, [&](auto v) { callback(v, startIndex++); });
}

template <typename Callback, typename IT, typename OnDoneCallback>
auto foreach_helper_pool(std::true_type, IT a, IT b, Callback callback, size_t startIndex = 0,
                         OnDoneCallback onTaskDone = []() {}) {
    return dispatchPool(
        [id = startIndex, c = std::move(callback), done = std::move(onTaskDone), a, b]() mutable {
            std::for_each(a, b, [&](auto v) { c(v, id++); });
            done();
        });
}

template <typename Callback, typename IT, typename OnDoneCallback>
auto foreach_helper_pool(std::false_type, IT a, IT b, Callback callback, size_t /*startIndex*/ = 0,
                         OnDoneCallback onTaskDone = []() {}) {
    return dispatchPool([c = std::move(callback), a, b]() {
        std::for_each(a, b, c);
        done();
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
template <typename Iterable, typename T = typename Iterable::value_type, typename Callback,
          typename OnDoneCallback = std::function<void()>>
std::vector<std::future<void>> forEachParallelAsync(const Iterable& iterable, Callback callback,
                                                    size_t jobs = 0,
                                                    OnDoneCallback onTaskDone = []() {}) {
    auto settings = InviwoApplication::getPtr()->getSettingsByType<SystemSettings>();
    auto poolSize = settings->poolSize_.get();
    using IncludeIndexType = typename std::conditional<util::is_callable_with<T, size_t>(callback),
                                                       std::true_type, std::false_type>::type;
    if (poolSize == 0) {
        detail::foreach_helper(IncludeIndexType(), iterable.begin(), iterable.end(), callback);
        onTaskDone();
        return {};
    }

    if (jobs == 0) {  // If jobs is zero, set to 4 times the pool size
        jobs = 4 * poolSize;
    }

    auto s = iterable.size();
    std::vector<std::future<void>> futures;
    for (size_t job = 0; job < jobs; ++job) {
        size_t start = (s * job) / jobs;
        size_t end = (s * (job + 1)) / jobs;
        auto a = iterable.begin() + start;
        auto b = iterable.begin() + end;
        futures.push_back(
            detail::foreach_helper_pool(IncludeIndexType(), a, b, callback, start, onTaskDone));
    }
    return futures;
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
template <typename Iterable, typename T = typename Iterable::value_type, typename Callback>
void forEachParallel(const Iterable& iterable, Callback callback, size_t jobs = 0) {
    auto futures = forEachParallelAsync<Iterable, T, Callback>(iterable, callback, jobs);

    for (const auto& e : futures) {
        e.wait();
    }
}

}  // namespace util

}  // namespace inviwo

#endif  // IVW_UTILITIES_H
