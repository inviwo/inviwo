/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/clock.h>
#include <inviwo/core/util/stringconversion.h>

namespace inviwo {

/** \class ClockGL
 *
 * Simple timer for OpenGL.
 * Uses OpenGL queries if OpenGL 3.3 or higher is supported,
 * otherwise uses CPU which calls glFinish before stopping time.
 * Usage is simplified by the macros (does nothing unless IVW_PROFILING is defined)
 * IVW_OPENGL_PROFILING("My message")
 */
class IVW_MODULE_OPENGL_API ClockGL {
public:
    ClockGL();
    virtual ~ClockGL();

    /** \brief Start OpenGL timing.
     */
    void start();

    /** \brief Stop OpenGL timing.
     */
    void stop();

    /** \brief Returns time taken for command execution in milliseconds.
     * @note Requires that stop has been called before.
     * @return float Time in milliseconds
     */
    float getElapsedTime() const;

private:
#ifdef GL_VERSION_3_3
    GLuint queries_[2];
#else
    Clock clock_;
#endif
};

/** \class ScopedClockGL
 *
 * Scoped timer for OpenGL that prints elapsed time in destructor.
 * Usage is simplified by the macros (does nothing unless IVW_PROFILING is defined)
 * IVW_OPENGL_PROFILING("My message")
 *
 */
class ScopedClockGL {
public:
    ScopedClockGL(const std::string& logSource, const std::string& message,
                  float logIfAtLeastMilliSec = 0.0f)
        : logSource_(logSource), logMessage_(message), logIfAtLeastMilliSec_(logIfAtLeastMilliSec) {
        clock_.start();
    }
    virtual ~ScopedClockGL() {
        clock_.stop();
        if (clock_.getElapsedTime() > logIfAtLeastMilliSec_) {
            std::stringstream message;
            message << logMessage_ << ": " << clock_.getElapsedTime() << " ms";
            LogCentral::getPtr()->log(logSource_, inviwo::Info, __FILE__, __FUNCTION__, __LINE__,
                                      message.str());
        }
    }

private:
    // Default constructor not allowed
    ScopedClockGL() {};
    ClockGL clock_;
    std::string logSource_;
    std::string logMessage_;
    float logIfAtLeastMilliSec_;
};

#if IVW_PROFILING
#define IVW_OPENGL_PROFILING(message)                                                 \
    std::ostringstream ADDLINE(__stream);                                             \
    ADDLINE(__stream) << message;                                                     \
    ScopedClockGL ADDLINE(__clock)(parseTypeIdName(std::string(typeid(this).name())), \
                                   ADDLINE(__stream).str());
#else
#define IVW_OPENGL_PROFILING(message)
#endif

#if IVW_PROFILING
#define IVW_OPENGL_PROFILING_IF(time, message)                                        \
    std::ostringstream ADDLINE(__stream);                                             \
    ADDLINE(__stream) << message;                                                     \
    ScopedClockGL ADDLINE(__clock)(parseTypeIdName(std::string(typeid(this).name())), \
                                   ADDLINE(__stream).str(), time);
#else
#define IVW_OPENGL_PROFILING_IF(time, message)
#endif

}  // namespace

#endif  // IVW_CLOCK_GL_H
