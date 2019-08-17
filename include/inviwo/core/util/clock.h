/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_CLOCK_H
#define IVW_CLOCK_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/logcentral.h>

#include <sstream>
#include <string>
#include <chrono>

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
template <typename Clock>
class ScopedClock : public Clock {
public:
    ScopedClock() = delete;

    ScopedClock(const std::string& logSource, const std::string& message,
                typename Clock::duration logIfAtLeast = typename Clock::duration{},
                LogLevel logLevel = LogLevel::Info);

    ScopedClock(const std::string& logSource, const std::string& message,
                double logIfAtLeastMilliSec, LogLevel logLevel = LogLevel::Info);

    ~ScopedClock() { print(); }

    /**
     * log the accumulated time but only if it is larger than the duration threshold (logIfAtLeast)
     * given in the constructor.
     */
    void print();

    /**
     * log the accumulated time but only if it is larger than the duration threshold (logIfAtLeast)
     * given in the constructor. Also resets the clock and restarts it.
     */
    void printAndReset();

private:
    const std::string logSource_;
    const std::string logMessage_;

    const typename Clock::duration logIfAtLeast_;
    const LogLevel logLevel_ = LogLevel::Info;
};

template <typename Clock>
ScopedClock<Clock>::ScopedClock(const std::string& logSource, const std::string& message,
                                typename Clock::duration logIfAtLeast, LogLevel logLevel)
    : logSource_{logSource}
    , logMessage_{message}
    , logIfAtLeast_{logIfAtLeast}
    , logLevel_(logLevel) {}

template <typename Clock>
ScopedClock<Clock>::ScopedClock(const std::string& logSource, const std::string& message,
                                double logIfAtLeastMilliSec, LogLevel logLevel)
    : ScopedClock(logSource, message,
                  std::chrono::duration_cast<typename Clock::duration>(
                      std::chrono::duration<double, std::chrono::milliseconds::period>(
                          logIfAtLeastMilliSec)),
                  logLevel) {}

template <typename Clock>
void ScopedClock<Clock>::print() {
    if (Clock::getElapsedTime() > logIfAtLeast_) {
        std::stringstream message;
        message << logMessage_ << ": " << msToString(Clock::getElapsedMilliseconds());
        LogCentral::getPtr()->log(logSource_, logLevel_, LogAudience::Developer, __FILE__,
                                  __FUNCTION__, __LINE__, message.str());
    }
}

template <typename Clock>
void ScopedClock<Clock>::printAndReset() {
    print();
    Clock::reset();
    Clock::start();
}

/**
 * \class inviwo::ScopedClockCPU
 * scoped clock for CPU time measurements
 *
 * \see IVW_CPU_PROFILING(message), IVW_CPU_PROFILING_CUSTOM(src, message)
 * \see IVW_CPU_PROFILING_IF(time, message), IVW_CPU_PROFILING_IF(time, src, message)
 */
using ScopedClockCPU = ScopedClock<Clock>;

#define IVW_ADDLINE_PART1(x, y) x##y
#define IVW_ADDLINE_PART2(x, y) IVW_ADDLINE_PART1(x, y)
#define IVW_ADDLINE(x) IVW_ADDLINE_PART2(x, __LINE__)

/**
 * \def IVW_CPU_PROFILING(message)
 * creates a scoped CPU clock with the given message.
 *
 * @param src      source of the log message
 */

/**
 * \def IVW_CPU_PROFILING_CUSTOM(src, message)
 * creates a scoped CPU clock with the given source and message.
 * Does nothing unless IVW_PROFILING is defined.
 *
 * @param src      source of the log message
 * @param message  log message
 */

/**
 * \def IVW_CPU_PROFILING_IF(time, message)
 * creates a scoped CPU clock with the given message and minimum duration.
 * Does nothing unless IVW_PROFILING is defined.
 *
 * @param time     either a std::chrono::duration or a double value (milliseconds)
 * @param message  log message
 */

/**
 * \def IVW_CPU_PROFILING_IF_CUSTOM(time, src, message)
 * creates a scoped CPU clock with the given source, message, and minimum duration.
 * Does nothing unless IVW_PROFILING is defined.
 *
 * @param time     either a std::chrono::duration or a double value (milliseconds)
 * @param src      source of the log message
 * @param message  log message
 */

#if IVW_PROFILING
#define IVW_CPU_PROFILING(message)                                                         \
    std::ostringstream IVW_ADDLINE(__stream);                                              \
    IVW_ADDLINE(__stream) << message;                                                      \
    ScopedClockCPU IVW_ADDLINE(__clock)(parseTypeIdName(std::string(typeid(this).name())), \
                                        IVW_ADDLINE(__stream).str());
#else
#define IVW_CPU_PROFILING(message)
#endif

#if IVW_PROFILING
#define IVW_CPU_PROFILING_CUSTOM(src, message) \
    std::ostringstream IVW_ADDLINE(__stream);  \
    IVW_ADDLINE(__stream) << message;          \
    ScopedClockCPU IVW_ADDLINE(__clock)(src, IVW_ADDLINE(__stream).str());
#else
#define IVW_CPU_PROFILING_CUSTOM(src, message)
#endif

#if IVW_PROFILING
#define IVW_CPU_PROFILING_IF(time, message)                                                \
    std::ostringstream IVW_ADDLINE(__stream);                                              \
    IVW_ADDLINE(__stream) << message;                                                      \
    ScopedClockCPU IVW_ADDLINE(__clock)(parseTypeIdName(std::string(typeid(this).name())), \
                                        IVW_ADDLINE(__stream).str(), time);
#else
#define IVW_CPU_PROFILING_IF(time, message)
#endif

#if IVW_PROFILING
#define IVW_CPU_PROFILING_IF_CUSTOM(time, src, message) \
    std::ostringstream IVW_ADDLINE(__stream);           \
    IVW_ADDLINE(__stream) << message;                   \
    ScopedClockCPU IVW_ADDLINE(__clock)(src, IVW_ADDLINE(__stream).str(), time);
#else
#define IVW_CPU_PROFILING_IF_CUSTOM(time, src, message)
#endif

}  // namespace inviwo

#endif  // IVW_CLOCK_H
