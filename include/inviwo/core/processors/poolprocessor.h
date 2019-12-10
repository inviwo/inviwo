/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwoapplication.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/activityindicator.h>
#include <inviwo/core/processors/progressbarowner.h>
#include <inviwo/core/util/timer.h>
#include <inviwo/core/network/processornetwork.h>

#include <atomic>
#include <chrono>

namespace inviwo {

class PoolProcessor;

namespace pool {

namespace detail {

struct Wrapper;
struct State;
template <typename Result, typename Done>
struct StateTemplate;

}  // namespace detail

/**
 * A class to signal if a background calculation should stop or be aborted.
 * Generally used by the background jobs to abort a calculation early:
 * ```{.cpp}
 * auto calc = [mystate](pool::Stop stop) {
 *     for(...) {
 *         if (stop) return nullptr;
 *         // do work
 *     }
 *     return results;
 * };
 * ```
 * \see PoolProcessor
 */
class IVW_CORE_API Stop {
public:
    operator bool() const noexcept { return stop_.load(); }

private:
    friend detail::State;
    Stop(const std::atomic<bool>& stop) : stop_{stop} {}
    const std::atomic<bool>& stop_;
};

/**
 * A class to signal the progress of a background calculation:
 * The progress reported should be in the range of [0.0, 1.0]. If the pool processor is running
 * multiple jobs the progress will automatically be normalized. Only the progress of the last
 * submitted job will be reported to the user.
 * ```{.cpp}
 * auto calc = [mystate](pool::Progress progress) {
 *     for(int i = n; i < N; ++i) {
 *         progress(i, N);
 *         // do work
 *     }
 *     return results;
 * };
 * ```
 * \see PoolProcessor
 */
class IVW_CORE_API Progress {
public:
    void operator()(float progress) const noexcept;
    void operator()(double progress) const noexcept;
    void operator()(size_t i, size_t max) const noexcept;

private:
    friend detail::State;
    Progress(detail::State& state, size_t id) : state_{state}, id_{id} {}
    detail::State& state_;
    const size_t id_;
};

/** @enum pool::Option
 * Settings for the PoolProcessor
 * \see PoolProcessor
 */
enum class Option {
    KeepOldResults =
        1 << 0,  ///< Also call done for old jobs, by default old jobs will be discarded.
    QueuedDispatch =
        1 << 1,  ///< Don't submit new jobs while old ones are running. The last submission will be
                 ///< queued and submitted when the current one is finished
    DelayDispatch =
        1 << 2,  ///< Wait for a small delay (500ms) of inactivity before submitting a job
    DelayInvalidation = 1 << 3  ///< Delay invalidation of outports until the job is finished. This
                                ///< will override the default processor invalidation.
};

}  // namespace pool

ALLOW_FLAGS_FOR_ENUM(pool::Option);
namespace pool {

/**
 * \copydoc pool::Option
 */
using Options = flags::flags<pool::Option>;
}  // namespace pool

/**
 * PoolProcessor is a base class to help make processors that dispatch work to the thread pool.
 */
class IVW_CORE_API PoolProcessor : public Processor, public ProgressBarOwner {
public:
    /**
     * Construct the pool processor with given options.
     * \see pool::Option
     */
    PoolProcessor(pool::Options options = pool::Options{flags::empty},
                  const std::string& identifier = "", const std::string& displayName = "");

    virtual ~PoolProcessor();

    /**
     * Cancel all current jobs
     */
    void stopJobs();

    /**
     * Are there any ongoing background jobs
     */
    bool hasJobs();

    /**
     * Dispatch a single background job. The job will be executed in a background thread in
     * the thread pool. It is important that the job captures its state by value, since it might
     * outlive the processor. The job can take two optional parameters
     *     * pool::Stop a stop token to periodically check if the job has been canceled.
     *     * pool::Progress a callback to report progress of the calculation. The progress is
     *       represented as a float in the interval [0.0, 1.0].
     * If the job takes a progress callback the processor will show a progress bar.
     * The second functor done is called with the result when the background job is finished. It
     * will be executed on the main thread, and only if the processor is still valid and the job has
     * not been stopped. Hence it is safe to refer to the processor in this functor.
     *
     * \code{.cpp}
     * const auto calc = [image = inport_.getData()]
     *     ( pool::Stop stop, pool::Progress progress) -> std::shared_ptr<const Image> {
     *     if (stop) return nullptr;
     *     auto newImage = std::shared_ptr<Image>(image->clone());
     *     progress(0.5f);
     *     // Do some work with the image
     *     return newImage;
     * };
     *
     * dispatchOne(calc, [this](std::shared_ptr<const Image> result) {
     *     outport_.setData(result);
     *     newResults();  // Let the network know that the processor has new results on the
     *                    // outport.
     * });
     * \endcode
     */
    template <typename Job, typename Done>
    void dispatchOne(Job&& job, Done&& done);

    /**
     * Dispatch a vector of background jobs. The jobs will be executed in a background thread in
     * the thread pool. It is important that the jobs captures its state by value, since it might
     * outlive the processor. The jobs can take two optional parameters
     *     * pool::Stop a stop token to periodically check if the job has been canceled.
     *     * pool::Progress a callback to report progress of the calculation. The progress is
     *       represented as a float in the interval [0.0, 1.0]. The progress is automatically
     *       normalized across all the jobs
     * If the jobs takes a progress callback the processor will show a progress bar. The progress
     * from all the jobs will automatically be merged and normalized
     *
     * The second functor 'done' is called with the results when the background jobs are finished.
     * It will be executed on the main thread, and only if the processor is still valid and the jobs
     * have not been stopped. Hence it is safe to refer to the processor in this functor.
     *
     * \code{.cpp}
     * std::vector<std::function<std::shared_ptr<Mesh>(pool::Stop, pool::Progress progress)>> jobs;
     * for (...) {
     *     jobs.push_back([some state](pool::Stop stop, pool::Progress progress) {
     *         if (stop) return nullptr;
     *         // construct some new Mesh
     *         progress(0.5f);
     *         return newMesh;
     *     });
     * }
     * dispatchMany(jobs, [this](std::vector<std::shared_ptr<Mesh>> results) {
     *     outport_.setData(std::make_shared<std::vector<std::shared_ptr<Mesh>>>(results));
     *     newResults();
     * });
     * \endcode
     */
    template <typename Job, typename Done>
    void dispatchMany(std::vector<Job> jobs, Done&& done);

    /**
     * handleError is called on the main thread whenever there has be an error in a background
     * calculation this will by default just log the error message, and clear any outports. Deriving
     * classes can overload this and rethrow the caught exception to enable custom error handling if
     * needed.
     */
    virtual void handleError();

    /**
     * Call newResults to let the network know we have new data on our outports.
     */
    void newResults();

    /**
     * Call newResults to let the network know we have new data on our outports.
     * Only invalidate the specified outports. Use this if you only set data to a subset of the
     * outports.
     */
    void newResults(const std::vector<Outport*>& outports);

    /**
     * Overrides Processor::invalidate to customize the invalidation behaviour
     * We generally only want to invalidate the dependent processor when we put data on our
     * outports.
     */
    virtual void invalidate(InvalidationLevel invalidationLevel,
                            Property* source = nullptr) override;

    /**
     * Get the current Options \see pool::Option
     */
    pool::Options getOptions() const { return options_; }

    /**
     *  \see pool::Option::KeepOldResults
     */
    bool keepOldJobs() const { return options_.contains(pool::Option::KeepOldResults); }
    /**
     *  \see pool::Option::QueuedDispatch
     */
    bool queuedDispatch() const { return options_.contains(pool::Option::QueuedDispatch); }
    /**
     *  \see pool::Option::DelayDispatch
     */
    bool delayDispatch() const { return options_.contains(pool::Option::DelayDispatch); }
    /**
     *  \see pool::Option::DelayInvalidation
     */
    bool delayInvalidation() const { return options_.contains(pool::Option::DelayInvalidation); }

private:
    friend pool::detail::State;

    struct Submission {
        std::shared_ptr<pool::detail::State> state;
        std::vector<std::function<void()>> tasks;
        std::function<void()> setupProgress;
    };

    void submit(Submission& job);

    template <typename Job>
    void setupProgress();

    void progress(pool::detail::State* state, float progress);

    template <typename Result, typename Done>
    std::shared_ptr<pool::detail::StateTemplate<Result, Done>> makeState(size_t count, Done&& done);

    bool removeState(const std::shared_ptr<pool::detail::State>& state);

    template <typename Result, typename Job>
    std::shared_ptr<std::packaged_task<Result()>> makeTask(Job&& job, pool::Stop stop,
                                                           pool::Progress progress);

    template <typename Result, typename Done>
    static void callDone(InviwoApplication* app,
                         std::shared_ptr<pool::detail::StateTemplate<Result, Done>> state);

    pool::Options options_;
    std::shared_ptr<pool::detail::Wrapper> wrapper_;
    std::vector<std::shared_ptr<pool::detail::State>> states_;
    std::vector<Submission> queue_;
    Delay delay_;
};

namespace pool::detail {
struct IVW_CORE_API Wrapper {
    Wrapper(PoolProcessor& pp) : processor(pp) {}
    PoolProcessor& processor;
};

struct IVW_CORE_API State {
    State(std::weak_ptr<Wrapper> processor, size_t count)
        : processor(processor), count{count}, stop{false}, progress(count) {}

    std::weak_ptr<Wrapper> processor;
    std::atomic<size_t> count;
    std::atomic<bool> stop;
    std::vector<std::atomic<float>> progress;
    std::future<void> progressUpdate;

    Stop getStop() { return Stop(stop); }

    void setProgress(size_t id, float progress);

    Progress getProgress(size_t id) {
        IVW_ASSERT(id < count, "Id has to be less than count to be valid");
        return Progress{*this, id};
    }
};

template <typename Result, typename Done>
struct StateTemplate : State {
    StateTemplate(std::weak_ptr<Wrapper> processor, size_t count, Done&& done)
        : State(processor, count), futures{}, done{std::forward<Done>(done)} {}

    std::vector<std::future<Result>> futures;
    Done done;
};

template <typename Job>
struct JobTraits {
    static_assert(std::is_invocable_v<Job> || std::is_invocable_v<Job, pool::Stop> ||
                  std::is_invocable_v<Job, Progress> || std::is_invocable_v<Job, Stop, Progress> ||
                  std::is_invocable_v<Job, Progress, Stop>);

    using CallTest =
        std::tuple<std::is_invocable<Job>, std::is_invocable<Job, Stop>,
                   std::is_invocable<Job, Progress>, std::is_invocable<Job, Stop, Progress>,
                   std::is_invocable<Job, Progress, Stop>>;

    using ValidArgs = std::tuple<std::tuple<>, std::tuple<Stop>, std::tuple<Progress>,
                                 std::tuple<Stop, Progress>, std::tuple<Progress, Stop>>;

    static constexpr size_t argIndex = util::index_of_derived<std::true_type, CallTest>();
    using Args = std::tuple_element_t<argIndex, ValidArgs>;
    using Result = decltype(std::apply(std::declval<Job>(), std::declval<Args>()));
};

}  // namespace pool::detail

template <typename Result, typename Done>
inline void PoolProcessor::callDone(
    InviwoApplication* app, std::shared_ptr<pool::detail::StateTemplate<Result, Done>> state) {
    static const auto done = [](PoolProcessor& p, auto state) {
        try {
            if constexpr (std::is_invocable_v<Done, Result>) {
                state->done(state->futures.front().get());
            } else {
                std::vector<Result> results;
                for (auto& res : state->futures) {
                    results.push_back(res.get());
                }
                state->done(results);
            }
        } catch (...) {
            p.handleError();
        }
    };

    if (state->count.fetch_sub(1) == 1) {
        app->dispatchFrontAndForget([state]() {
            if (auto wrapper = state->processor.lock()) {
                auto& p = wrapper->processor;
                const bool isLast = p.removeState(state);
                p.getProgressBar().setActive(p.hasJobs());
                if (isLast) p.getProgressBar().hide();

                if (p.queuedDispatch() && !p.queue_.empty()) {
                    p.submit(p.queue_.front());
                    p.queue_.clear();
                    return;
                }

                if (state->stop) return;

                if (isLast || p.keepOldJobs()) {
                    done(p, state);
                }
            }
        });
    }
}

template <typename Job, typename Done>
void PoolProcessor::dispatchMany(std::vector<Job> jobs, Done&& done) {
    using Result = typename pool::detail::JobTraits<Job>::Result;

    static_assert(std::is_invocable_v<Done, std::vector<Result>>);

    if (!keepOldJobs()) stopJobs();

    auto state = makeState<Result, Done>(jobs.size(), std::forward<Done>(done));
    Submission sub{state, {}, [this]() { setupProgress<Job>(); }};

    auto app = getNetwork()->getApplication();
    size_t i = 0;
    for (auto& job : jobs) {
        auto task = makeTask<Result>(std::move(job), state->getStop(), state->getProgress(i++));
        state->futures.push_back(task->get_future());
        sub.tasks.emplace_back([state, task, app]() {
            if (!state->stop) (*task)();
            callDone(app, state);
        });
    }

    if (delayDispatch()) {
        queue_.clear();
        queue_.push_back(std::move(sub));
        delay_.start();

    } else if (queuedDispatch() && !states_.empty()) {
        queue_.clear();
        queue_.push_back(std::move(sub));

    } else {
        submit(sub);
    }
}

template <typename Job, typename Done>
void PoolProcessor::dispatchOne(Job&& job, Done&& done) {
    using Result = typename pool::detail::JobTraits<Job>::Result;

    static_assert(std::is_invocable_v<Done, Result>);

    if (!keepOldJobs()) stopJobs();

    auto state = makeState<Result, Done>(1, std::forward<Done>(done));
    auto task = makeTask<Result>(std::forward<Job>(job), state->getStop(), state->getProgress(0));
    state->futures.push_back(task->get_future());
    auto app = getNetwork()->getApplication();

    Submission sub{state,
                   {[state, task, app]() {
                       if (!state->stop) (*task)();
                       callDone(app, state);
                   }},
                   [this]() { setupProgress<Job>(); }};

    if (delayDispatch()) {
        queue_.clear();
        queue_.push_back(std::move(sub));
        delay_.start();

    } else if (queuedDispatch() && !states_.empty()) {
        queue_.clear();
        queue_.push_back(std::move(sub));

    } else {
        submit(sub);
    }
}

template <typename Job>
inline void PoolProcessor::setupProgress() {
    updateProgress(0.0f);
    getProgressBar().setActive(true);
    if constexpr (std::is_invocable_v<Job, pool::Progress> ||
                  std::is_invocable_v<Job, pool::Stop, pool::Progress> ||
                  std::is_invocable_v<Job, pool::Progress, pool::Stop>) {
        getProgressBar().show();
    }
}

template <typename Result, typename Done>
inline std::shared_ptr<pool::detail::StateTemplate<Result, Done>> PoolProcessor::makeState(
    size_t count, Done&& done) {
    return std::make_shared<pool::detail::StateTemplate<Result, Done>>(wrapper_, count,
                                                                       std::forward<Done>(done));
}

template <typename Result, typename Job>
inline std::shared_ptr<std::packaged_task<Result()>> PoolProcessor::makeTask(
    Job&& job, [[maybe_unused]] pool::Stop stop, [[maybe_unused]] pool::Progress progress) {
    if constexpr (std::is_invocable_v<Job, pool::Stop, pool::Progress>) {
        return std::make_shared<std::packaged_task<Result()>>(
            [job = std::forward<Job>(job), stop, progress]() { return job(stop, progress); });
    } else if constexpr (std::is_invocable_v<Job, pool::Progress, pool::Stop>) {
        return std::make_shared<std::packaged_task<Result()>>(
            [job = std::forward<Job>(job), stop, progress]() { return job(progress, stop); });
    } else if constexpr (std::is_invocable_v<Job, pool::Stop>) {
        return std::make_shared<std::packaged_task<Result()>>(
            [job = std::forward<Job>(job), stop]() { return job(stop); });
    } else if constexpr (std::is_invocable_v<Job, pool::Progress>) {
        return std::make_shared<std::packaged_task<Result()>>(
            [job = std::forward<Job>(job), progress]() { return job(progress); });
    } else {
        return std::make_shared<std::packaged_task<Result()>>(std::forward<Job>(job));
    }
}

}  // namespace inviwo
