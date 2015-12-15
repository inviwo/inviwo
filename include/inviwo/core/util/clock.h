/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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
#include <string>
#include <chrono>

namespace inviwo {

/** \class Clock
 *
 * Class for measure time.
 */
class IVW_CORE_API Clock {
public:
    Clock();
    virtual ~Clock() {}

    /**
     * Start the Clock.
     *
     */
    void start();

    /**
     * Set new reference time.
     *
     */
    void tick();

    /**
    * Returns the amount of milliseconds between start and tick call.
    * If neither start or tick has been called, the return value is undefined.
    */
    float getElapsedMiliseconds() const;

    /**
    * Returns the amount of seconds between start and tick call.
    * If neither start or tick has been called, the return value is undefined.
    */
    float getElapsedSeconds() const;

protected:
    std::chrono::high_resolution_clock::time_point startTime_;
    std::chrono::high_resolution_clock::time_point tickTime_;
};

/** \class ScopedClockCPU
 *
 * Scoped timer for CPU that prints elapsed time in destructor.
 * Usage is simplified by the macros (does nothing unless IVW_PROFILING is defined)
 * IVW_CPU_PROFILING("My message")
 *
 */
class IVW_CORE_API ScopedClockCPU {
public:
    ScopedClockCPU(const std::string& logSource, const std::string& message,
                   float logIfAtLeastMilliSec = 0.0f)
        : logSource_(logSource), logMessage_(message), logIfAtLeastMilliSec_(logIfAtLeastMilliSec) {
        clock_.start();
    }

    void print();
    void reset();
    void printAndReset();

    virtual ~ScopedClockCPU();

private:
    // Default constructor not allowed
    // ScopedClockCPU() {};
    Clock clock_;

// We can safely ingnore the C4251 warning for private members.
#pragma warning( push )
#pragma warning( disable: 4251 )
    std::string logSource_;
    std::string logMessage_;
#pragma warning( pop )

    float logIfAtLeastMilliSec_;
};

#define ADDLINE_PART1(x, y) x##y
#define ADDLINE_PART2(x, y) ADDLINE_PART1(x, y)
#define ADDLINE(x) ADDLINE_PART2(x, __LINE__)

#if IVW_PROFILING
#define IVW_CPU_PROFILING(message)                                                     \
    std::ostringstream ADDLINE(__stream);                                              \
    ADDLINE(__stream) << message;                                                      \
    ScopedClockCPU ADDLINE(__clock)(parseTypeIdName(std::string(typeid(this).name())), \
                                    ADDLINE(__stream).str());
#else
#define IVW_CPU_PROFILING(message)
#endif

#if IVW_PROFILING
#define IVW_CPU_PROFILING_IF(time, message)                                            \
    std::ostringstream ADDLINE(__stream);                                              \
    ADDLINE(__stream) << message;                                                      \
    ScopedClockCPU ADDLINE(__clock)(parseTypeIdName(std::string(typeid(this).name())), \
                                    ADDLINE(__stream).str(), time);
#else
#define IVW_CPU_PROFILING_IF(time, message)
#endif

};  // namespace inviwo

#endif  // IVW_TIMER_H