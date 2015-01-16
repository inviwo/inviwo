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

#include <modules/opengl/clockgl.h>


namespace inviwo {

#if GL_VERSION_3_3 && GL_ARB_timer_query
ClockGL::ClockGL() {
    // Note that this can take
    // Generate start and end queries
    glGenQueries(2, queries_);
}

ClockGL::~ClockGL() {
    // Release generated queries
    glDeleteQueries(2, queries_);
}

void ClockGL::start() {
    // Start query
    glQueryCounter(queries_[0], GL_TIMESTAMP);
}

void ClockGL::stop() {
    // Set end query
    glQueryCounter(queries_[1], GL_TIMESTAMP);
}

float ClockGL::getElapsedTime() const {
    int done = 0;
    // Wait for results to be available
    while (!done) {
        glGetQueryObjectiv(queries_[1], GL_QUERY_RESULT_AVAILABLE, &done);
    }
    GLuint64 start;
    GLuint64 stop;
    glGetQueryObjectui64v(queries_[0], GL_QUERY_RESULT, &start);
    glGetQueryObjectui64v(queries_[1], GL_QUERY_RESULT, &stop);
    return 1e-6f * static_cast<float>(stop-start);
}
#else
// OpenGL 3.3 is not supported, use CPU clock as fallback
ClockGL::ClockGL() {}
ClockGL::~ClockGL() {}
void ClockGL::start() { clock_.start(); }
void ClockGL::stop() {
    glFinish(); 
    clock_.stop();
}
float ClockGL::getElapsedTime() const { return clock_.getElapsedMiliseconds(); }

#endif


} // namespace
