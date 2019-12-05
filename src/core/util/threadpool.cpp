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

#include <inviwo/core/util/threadpool.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

// the constructor just launches some amount of workers
ThreadPool::ThreadPool(size_t threads, std::function<void()> onThreadStart,
                       std::function<void()> onThreadStop)
    : onThreadStart_{std::move(onThreadStart)}, onThreadStop_{std::move(onThreadStop)} {
    while (workers.size() < threads) {
        workers.push_back(std::make_unique<Worker>(*this));
    }
}

size_t ThreadPool::trySetSize(size_t size) {
    while (workers.size() < size) {
        workers.push_back(std::make_unique<Worker>(*this));
    }

    if (workers.size() > size) {
        auto active = workers.size();
        for (auto& worker : workers) {
            auto exprected = State::Free;
            if (worker->state.compare_exchange_strong(exprected, State::Stop)) {
                --active;
            } else if (exprected == State::Stop || exprected == State::Done) {
                --active;
            }
            if (active <= size) break;
        }

        condition.notify_all();

        util::erase_remove_if(
            workers, [](std::unique_ptr<Worker>& worker) { return worker->state == State::Done; });
    }
    return workers.size();
}

size_t ThreadPool::getSize() const { return workers.size(); }

size_t ThreadPool::getQueueSize() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    return tasks.size();
}

ThreadPool::~ThreadPool() {
    for (auto& worker : workers) worker->state = State::Abort;
    condition.notify_all();
    workers.clear();  // this will join all threads.
}

ThreadPool::Worker::~Worker() { thread.join(); }

ThreadPool::Worker::Worker(ThreadPool& pool)
    : state{State::Free}, thread{[this, &pool]() {
        pool.onThreadStart_();
        util::OnScopeExit cleanup{[&pool]() { pool.onThreadStop_(); }};

        for (;;) {
            std::function<void()> task;
            state = State::Free;
            {
                std::unique_lock<std::mutex> lock(pool.queue_mutex);
                pool.condition.wait(lock, [this, &pool] {
                    return state == State::Abort || state == State::Stop || !pool.tasks.empty();
                });
                if (state == State::Abort || (state == State::Stop && pool.tasks.empty())) break;
                task = std::move(pool.tasks.front());
                pool.tasks.pop();
            }
            state = State::Working;
            try {
                task();
            } catch (...) {  // Make sure we don't leak any exceptions.
            }
        }
        state = State::Done;
    }} {}

void ThreadPool::enqueueRaw(std::function<void()> task) {
    if (workers.empty()) {
        task();  // No worker threads, just run the task.
    } else {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.emplace(std::move(task));
    }
    condition.notify_one();
}

}  // namespace inviwo
