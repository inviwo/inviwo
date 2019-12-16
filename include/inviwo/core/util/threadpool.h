/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

/*********************************************************************************
 *
 * Copyright (c) 2012 Jakob Progsch, Václav Zeman
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 *
 *********************************************************************************/

// following https://github.com/progschj/ThreadPool

#ifndef IVW_THREADPOOL_H
#define IVW_THREADPOOL_H

#include <inviwo/core/common/inviwocoredefine.h>

#include <warn/push>
#include <warn/ignore/all>
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <atomic>
#include <warn/pop>

namespace inviwo {

class IVW_CORE_API ThreadPool {
public:
    ThreadPool(size_t threads, std::function<void()> onThreadStart = []() {},
               std::function<void()> onThreadStop = []() {});
    ~ThreadPool();

    /**
     * Enqueue function f with arguments args. The function f may throw exceptions.
     * @return a future to the result of f
     */
    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

    /**
     * Enqueue a plain functor. The functor may not throw exceptions.
     */
    void enqueueRaw(std::function<void()> f);

    size_t trySetSize(size_t size);
    size_t getSize() const;

    size_t getQueueSize();

private:
    enum class State {
        Free,     //< Worker is waiting for tasks.
        Working,  //< Worker is running a task.
        Stop,     //< Stop after all tasks are done.
        Abort,    //< Stop as soon as possible, no matter if there are more tasks.
        Done      //< Worker is waiting to be joined.
    };

    struct Worker {
        Worker(ThreadPool& pool);
        Worker(const Worker&) = delete;
        Worker(Worker&& rhs) = delete;
        Worker& operator=(const Worker&) = delete;
        Worker& operator=(Worker&& rhs) = delete;
        ~Worker();

        std::atomic<State> state;  //< State of the worker
        std::thread thread;
    };
    // need to keep track of threads so we can join them
    std::vector<std::unique_ptr<Worker>> workers;

    // the task queue
    std::queue<std::function<void()>> tasks;

    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;

    // Thread start end exit actions
    std::function<void()> onThreadStart_;
    std::function<void()> onThreadStop_;
};

// add new work item to the pool
template <class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();

    if (workers.empty()) {
        (*task)();  // No worker threads, just run the task.
    } else {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return res;
}

}  // namespace inviwo

#endif  // IVW_THREADPOOL_H
