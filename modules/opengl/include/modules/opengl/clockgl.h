/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#ifndef IVW_CLOCK_GL_H
#define IVW_CLOCK_GL_H

#include <modules/opengl/openglmoduledefine.h>
#include <modules/opengl/inviwoopengl.h>
#include <inviwo/core/util/clock.h>

#include <chrono>
#include <vector>
#include <array>

namespace inviwo {

/** \class ClockGL
 *
 * Clock for measuring elapsed time between a start and stop points on the GPU using OpenGL
 * performance timer queries. The clock accumulates the elapsed times when start and stop are called
 * multiple times.
 *
 * Note: The render context of the current thread needs to be activated. Using the same ClockGL
 *       instance in different render contexts results in undefined behavior and getElapsedTime()
 *       will most likely time out.
 *
 * \see RenderContext::activateDefaultRenderContext, RenderContext::activateLocalRenderContext
 */
class IVW_MODULE_OPENGL_API ClockGL {
public:
    using duration = std::chrono::nanoseconds;

    /**
     * creates a clock and starts it
     */
    ClockGL();

    ClockGL(const ClockGL&) = delete;
    ClockGL(ClockGL&&) = default;

    ClockGL& operator=(const ClockGL&) = delete;
    ClockGL& operator=(ClockGL&&) = default;

    ~ClockGL();

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
     * resets the queries and sets the accumulated time and count to 0
     */
    void reset();

    /**
     * return the number of times start has been called. Useful for calculating averages.
     */

    size_t getCount() const;

    /**
     * returns the accumulated time. If the clock is running the result is accumulated
     * time plus the current elapsed time. This function will wait at most \p timeout seconds
     * for the performance queries to be completed. In case of a time out, the results are
     * incorrect.
     *
     * @param timeout    time out in seconds when querying the OpenGL performance counters
     * @return accumulated time
     */
    duration getElapsedTime(std::chrono::seconds timeout = std::chrono::seconds{30});

    /**
     * returns the accumulated time divided by count. If the clock is running the result is
     * accumulated time plus the current elapsed time divided by count. This function will wait at
     * most \p timeout seconds for the performance queries to be completed. In case of a time out,
     * the results are incorrect.
     *
     * @param timeout    time out in seconds when querying the OpenGL performance counters
     * @return accumulated time
     */
    duration getAverageElapsedTime(std::chrono::seconds timeout = std::chrono::seconds{30});

    /**
     * returns the accumulated time. If the clock is running the result is accumulated
     * time plus the current elapsed time. This function will wait at most \p timeout seconds
     * for the performance queries to be completed. In case of a time out, the results are
     * incorrect.
     *
     * @param timeout    time out in seconds when querying the OpenGL performance counters
     * @return accumulated time in milliseconds
     * \see getElapsedTime
     */
    double getElapsedMilliseconds(std::chrono::seconds timeout = std::chrono::seconds{30});

    /**
     * returns the accumulated time. If the clock is running the result is accumulated
     * time plus the current elapsed time. This function will wait at most \p timeout seconds
     * for the performance queries to be completed. In case of a time out, the results are
     * incorrect.
     *
     * @param timeout    time out in seconds when querying the OpenGL performance counters
     * @return accumulated time in seconds
     * \see getElapsedTime
     */
    double getElapsedSeconds(std::chrono::seconds timeout = std::chrono::seconds{30});

protected:
    void collectTiming();

    enum class State { Unused, Started, Stopped };
    struct Query {
        std::array<GLuint, 2> ids;
        State state;
        GLuint startId() const { return ids[0]; }
        GLuint stopId() const { return ids[1]; }
    };
    duration accumulatedTime_ = static_cast<duration>(0);
    size_t count_ = 0;
    std::vector<Query> queries_;
};

/**
 * \class inviwo::ScopedClockGL
 * scoped clock for OpenGL time measurements
 *
 * \see IVW_OPENGL_PROFILING(message), IVW_OPENGL_PROFILING_CUSTOM(src, message)
 * \see IVW_OPENGL_PROFILING_IF(time, message), IVW_OPENGL_PROFILING_IF(time, src, message)
 */
using ScopedClockGL = ScopedClock<ClockGL>;

/**
 * \def IVW_OPENGL_PROFILING(message)
 * creates a scoped ClockGL clock with the given message.
 *
 * @param src      source of the log message
 */

/**
 * \def IVW_OPENGL_PROFILING_CUSTOM(src, message)
 * creates a scoped ClockGL clock with the given source and message.
 * Does nothing unless IVW_PROFILING is defined.
 *
 * @param src      source of the log message
 * @param message  log message
 */

/**
 * \def IVW_OPENGL_PROFILING_IF(time, message)
 * creates a scoped ClockGL clock with the given message and minimum duration.
 * Does nothing unless IVW_PROFILING is defined.
 *
 * @param time     either a std::chrono::duration or a double value (milliseconds)
 * @param message  log message
 */

/**
 * \def IVW_OPENGL_PROFILING_IF_CUSTOM(time, src, message)
 * creates a scoped ClockGL clock with the given source, message, and minimum duration.
 * Does nothing unless IVW_PROFILING is defined.
 *
 * @param time     either a std::chrono::duration or a double value (milliseconds)
 * @param src      source of the log message
 * @param message  log message
 */

#if IVW_PROFILING
#define IVW_OPENGL_PROFILING(message)                                                     \
    std::ostringstream IVW_ADDLINE(__stream);                                             \
    IVW_ADDLINE(__stream) << message;                                                     \
    ScopedClockGL IVW_ADDLINE(__clock)(parseTypeIdName(std::string(typeid(this).name())), \
                                       IVW_ADDLINE(__stream).str());
#else
#define IVW_OPENGL_PROFILING(message)
#endif

#if IVW_PROFILING
#define IVW_OPENGL_PROFILING_CUSTOM(src, message) \
    std::ostringstream IVW_ADDLINE(__stream);     \
    IVW_ADDLINE(__stream) << message;             \
    ScopedClockGL IVW_ADDLINE(__clock)(src, IVW_ADDLINE(__stream).str());
#else
#define IVW_OPENGL_PROFILING_CUSTOM(src, message)
#endif

#if IVW_PROFILING
#define IVW_OPENGL_PROFILING_IF(time, message)                                            \
    std::ostringstream IVW_ADDLINE(__stream);                                             \
    IVW_ADDLINE(__stream) << message;                                                     \
    ScopedClockGL IVW_ADDLINE(__clock)(parseTypeIdName(std::string(typeid(this).name())), \
                                       IVW_ADDLINE(__stream).str(), time);
#else
#define IVW_OPENGL_PROFILING_IF(time, message)
#endif

#if IVW_PROFILING
#define IVW_OPENGL_PROFILING_IF_CUSTOM(time, src, message) \
    std::ostringstream IVW_ADDLINE(__stream);              \
    IVW_ADDLINE(__stream) << message;                      \
    ScopedClockGL IVW_ADDLINE(__clock)(src, IVW_ADDLINE(__stream).str(), time);
#else
#define IVW_OPENGL_PROFILING_IF_CUSTOM(time, src, message)
#endif

}  // namespace inviwo

#endif  // IVW_CLOCK_GL_H
