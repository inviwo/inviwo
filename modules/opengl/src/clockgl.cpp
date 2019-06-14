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

#include <modules/opengl/clockgl.h>

#include <inviwo/core/util/logcentral.h>

namespace inviwo {

ClockGL::ClockGL() { start(); }

ClockGL::~ClockGL() {
    for (auto& query : queries_) {
        glDeleteQueries(2, query.ids.data());
    }
}

bool ClockGL::isRunning() const {
    return !queries_.empty() && queries_.back().state == State::Started;
}

void ClockGL::start() {
    ++count_;
    collectTiming();

    if (!queries_.empty() && queries_.back().state == State::Started) {
        glQueryCounter(queries_.back().startId(), GL_TIMESTAMP);
    } else if (!queries_.empty() && queries_.front().state == State::Unused) {
        std::rotate(queries_.begin(), queries_.begin() + 1, queries_.end());
        glQueryCounter(queries_.back().startId(), GL_TIMESTAMP);
        queries_.back().state = State::Started;
    } else {
        queries_.push_back({{0, 0}, State::Started});
        glGenQueries(2, queries_.back().ids.data());
        glQueryCounter(queries_.back().startId(), GL_TIMESTAMP);
    }
}

void ClockGL::stop() {
    if (isRunning()) {
        glQueryCounter(queries_.back().stopId(), GL_TIMESTAMP);
        queries_.back().state = State::Stopped;
    }
}

void ClockGL::reset() {
    for (auto& query : queries_) {
        query.state = State::Unused;
    }
    accumulatedTime_ = static_cast<duration>(0);
    count_ = 0;
}

size_t ClockGL::getCount() const { return count_; }

auto ClockGL::getElapsedTime(std::chrono::seconds timeout) -> duration {
    auto startWaitingTime = std::chrono::high_resolution_clock::now();

    if (isRunning()) {
        // add query end marker to get time elapsed since start without affecting ongoing
        // measurement
        glQueryCounter(queries_.back().stopId(), GL_TIMESTAMP);
    }

    // ensure that query results are available. Abort after timeOut, which might be caused by
    // using the same ClockGL in different contexts or a GL computation taking too long.
    int done = 0;
    while (!done && std::chrono::high_resolution_clock::now() - startWaitingTime < timeout) {
        glGetQueryObjectiv(queries_.back().stopId(), GL_QUERY_RESULT_AVAILABLE, &done);
    }

    collectTiming();
    if (done) {
        if (isRunning()) {
            // special case if last query is still ongoing, i.e. this query is
            // not considered by collectTiming()
            GLuint64 start;
            GLuint64 stop;
            glGetQueryObjectui64v(queries_.back().startId(), GL_QUERY_RESULT, &start);
            glGetQueryObjectui64v(queries_.back().stopId(), GL_QUERY_RESULT, &stop);
            return accumulatedTime_ + std::chrono::nanoseconds{stop - start};
        } else {
            return accumulatedTime_;
        }
    } else {  // timeout
        std::string msg =
            "ClockGL timeout: ClockGL waited " +
            std::to_string(std::chrono::duration_cast<std::chrono::seconds>(timeout).count()) +
            "s for OpenGL to finish. Elapsed time will be incorrect.\n"
            "This can be caused by compute-intensive operations or using "
            "ClockGL in different GL contexts.";
        LogCentral::getPtr()->log("ClockGL", LogLevel::Error, LogAudience::User, __FILE__,
                                  __FUNCTION__, __LINE__, msg);
        return accumulatedTime_;
    }
}

auto ClockGL::getAverageElapsedTime(std::chrono::seconds timeout) -> duration {
    return count_ > 0 ? duration{getElapsedTime(timeout).count() / count_} : duration{0};
}

void ClockGL::collectTiming() {
    for (auto& query : queries_) {
        if (query.state == State::Stopped) {
            int done = 0;
            glGetQueryObjectiv(query.stopId(), GL_QUERY_RESULT_AVAILABLE, &done);
            if (done) {
                GLuint64 start;
                GLuint64 stop;
                glGetQueryObjectui64v(query.startId(), GL_QUERY_RESULT, &start);
                glGetQueryObjectui64v(query.stopId(), GL_QUERY_RESULT, &stop);
                accumulatedTime_ += std::chrono::nanoseconds{stop - start};
                query.state = State::Unused;
            }
        }
    }
    std::stable_partition(queries_.begin(), queries_.end(),
                          [](auto& query) { return query.state == State::Unused; });
}

double ClockGL::getElapsedMilliseconds(std::chrono::seconds timeout) {
    using duration_double = std::chrono::duration<double, std::chrono::milliseconds::period>;
    return std::chrono::duration_cast<duration_double>(getElapsedTime(timeout)).count();
}

double ClockGL::getElapsedSeconds(std::chrono::seconds timeout) {
    using duration_double = std::chrono::duration<double, std::chrono::seconds::period>;
    return std::chrono::duration_cast<duration_double>(getElapsedTime(timeout)).count();
}

}  // namespace inviwo
