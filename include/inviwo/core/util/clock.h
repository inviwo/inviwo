/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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
#include <inviwo/core/util/chronoutils.h>
#include <inviwo/core/util/demangle.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/sourcecontext.h>

#include <sstream>
#include <string>
#include <chrono>
#include <fmt/chrono.h>

namespace inviwo {

/** \class Clock
 *
 * Clock for measuring elapsed time between a start and stop point. The clock accumulates the
 * elapsed times when start and stop are called multiple times.
 */
class IVW_CORE_API Clock {
public:
    using clock = std::chrono::high_resolution_clock;
    using duration = std::chrono::high_resolution_clock::duration;
    using time_point = std::chrono::high_resolution_clock::time_point;

    /**
     * creates a clock and starts it
     */
    Clock();

    /**
     * query whether the clock has been started
     *
     * @return true if the clock is running
     */
    bool isRunning() const;

    /**
     * starts the clock
     */
    void start();

    /**
     * stops the clock and accumulates the elapsed time since start() was called
     */
    void stop();

    /**
     * resets the accumulated time to 0
     */
    void reset();

    /**
     * returns the accumulated time. If the clock is running the result is accumulated
     * time plus the current elapsed time.
     *
     * @return accumulated time
     */
    duration getElapsedTime() const;

    /**
     * returns the accumulated time. If the clock is running the result is accumulated
     * time plus the current elapsed time.
     *
     * @return accumulated time in milliseconds
     * \see getElapsedTime
     */
    double getElapsedMilliseconds() const;

    /**
     * returns the accumulated time. If the clock is running the result is accumulated
     * time plus the current elapsed time.
     *
     * @return accumulated time in seconds
     * \see getElapsedTime
     */
    double getElapsedSeconds() const;

protected:
    bool isRunning_ = false;

    time_point startTime_;
    duration accumulatedTime_ = static_cast<duration>(0);
};

/** \class ScopedClock
 *
 * Scoped clock which prints the elapsed time when the instance is destroyed, i.e. print() is called
 * by the destructor.
 *
 * \see ScopedClockCPU, ScopedClockGL
 */
template <typename Clock, typename Callback>
class ScopedClock : public Clock {
public:
    using Duration = Clock::duration;
    ScopedClock() = delete;

    ScopedClock(Callback message, Duration logIfAtLeast = {}, LogLevel logLevel = LogLevel::Info,
                SourceContext context = std::source_location::current());

    ~ScopedClock() { print(); }

    /**
     * log the accumulated time but only if it is larger than the duration threshold (logIfAtLeast)
     * given in the constructor.
     */
    void print() const;

    /**
     * log the accumulated time but only if it is larger than the duration threshold (logIfAtLeast)
     * given in the constructor. Also resets the clock and restarts it.
     */
    void printAndReset();

private:
    SourceContext context_;
    Callback message_;

    typename Clock::duration logIfAtLeast_;
    LogLevel logLevel_ = LogLevel::Info;
};

namespace util {

template <typename Clock, typename Callback>
[[nodiscard]] auto makeScopedClock(Callback callback, typename Clock::duration logIfAtLeast = {},
                                   LogLevel logLevel = LogLevel::Info,
                                   SourceContext context = std::source_location::current())
    -> ScopedClock<Clock, Callback> {
    return ScopedClock<Clock, Callback>{callback, logIfAtLeast, logLevel, context};
}

template <typename Clock = Clock>
[[nodiscard]] auto makeScopedClock(Literal message, typename Clock::duration logIfAtLeast = {},
                                   LogLevel logLevel = LogLevel::Info,
                                   SourceContext context = std::source_location::current()) {
    const auto callback = [message]() { return message; };
    return ScopedClock<Clock, decltype(callback)>{callback, logIfAtLeast, logLevel, context};
}

}  // namespace util

template <typename Clock, typename Callback>
ScopedClock<Clock, Callback>::ScopedClock(Callback message, Duration logIfAtLeast,
                                          LogLevel logLevel, SourceContext context)
    : context_{context}, message_{message}, logIfAtLeast_{logIfAtLeast}, logLevel_(logLevel) {}

template <typename Clock, typename Callback>
void ScopedClock<Clock, Callback>::print() const {
    if (Clock::getElapsedTime() > logIfAtLeast_) {
        ::inviwo::log::report(
            logLevel_, context_, "{}: {}", message_(),
            std::chrono::duration_cast<std::chrono::milliseconds>(Clock::getElapsedTime()));
    }
}

template <typename Clock, typename Callback>
void ScopedClock<Clock, Callback>::printAndReset() {
    print();
    Clock::reset();
    Clock::start();
}

/**
 * scoped clock for CPU time measurements
 *
 * \see IVW_CPU_PROFILING(message), IVW_CPU_PROFILING_IF(time, message)
 */
template <typename Callback>
using ScopedClockCPU = ScopedClock<Clock, Callback>;

#define IVW_ADDLINE_PART1(x, y) x##y
#define IVW_ADDLINE_PART2(x, y) IVW_ADDLINE_PART1(x, y)
#define IVW_ADDLINE(x) IVW_ADDLINE_PART2(x, __LINE__)

/**
 * \def IVW_CPU_PROFILING(message)
 * creates a scoped CPU clock with the given message.
 *
 * @param message  log message
 */
#if IVW_PROFILING
#define IVW_CPU_PROFILING(message)                                                  \
    const auto IVW_ADDLINE(inviwoScopedClock) = util::makeScopedClock<Clock>([&]() { \
        std::ostringstream ss;                                                      \
        ss << message;                                                              \
        return std::move(ss).str();                                                 \
    })
#else
#define IVW_CPU_PROFILING(message)
#endif

/**
 * \def IVW_CPU_PROFILING_IF(time, message)
 * creates a scoped CPU clock with the given message and minimum duration.
 * Does nothing unless IVW_PROFILING is defined.
 *
 * @param time     a double value (milliseconds)
 * @param message  log message
 */
#if IVW_PROFILING
#define IVW_CPU_PROFILING_IF(time, message)                                   \
    const auto IVW_ADDLINE(inviwoScopedClock) = util::makeScopedClock<Clock>( \
        [&]() {                                                                \
            std::ostringstream ss;                                            \
            ss << message;                                                    \
            return std::move(ss).str();                                       \
        },                                                                    \
        std::chrono::milliseconds(time))
#else
#define IVW_CPU_PROFILING_IF(time, message)
#endif

}  // namespace inviwo
