/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2020 Inviwo Foundation
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

#include <modules/opengl/inviwoopengl.h>
#include <inviwo/core/util/logcentral.h>
#include <sstream>

namespace inviwo {

std::string getGLErrorString(GLenum err) {
    if (err == GL_NO_ERROR) {
        return "No error";
    }
#ifdef GLEW_NO_GLU
    std::string errorString = "";
    switch (err) {
        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated "
                   "argument. "
                   "The offending command is ignored and has no other side effect than to set the "
                   "error flag.";
        case GL_INVALID_VALUE:
            return "INVALID_VALUE: A numeric argument is out of range. The offending command is "
                   "ignored and has no other side effect than to set the error flag.";
        case GL_INVALID_OPERATION:
            return "INVALID_OPERATION: The specified operation is not allowed in the current "
                   "state. "
                   "The offending command is ignored and has no other side effect than to set the "
                   "error flag.";
        case GL_STACK_OVERFLOW:
            return "GL_STACK_OVERFLOW: This command would cause a stack overflow. The offending "
                   "command is ignored and has no other side effect than to set the error flag.";
        case GL_STACK_UNDERFLOW:
            return "GL_STACK_UNDERFLOW: This command would cause a stack underflow. The offending "
                   "command is ignored and has no other side effect than to set the error flag.";
        case GL_OUT_OF_MEMORY:
            return "OUT_OF_MEMORY: There is not enough memory left to execute the command. The "
                   "state "
                   "of the GL is undefined, except for the state of the error flags, after this "
                   "error "
                   "is recorded.";
        case GL_TABLE_TOO_LARGE:
            return "GL_TABLE_TOO_LARGE: The specified table exceeds the implementation's maximum "
                   "supported table size. The offending command is ignored and has no other side "
                   "effect than to set the error flag.";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "INVALID_FRAMEBUFFER_OPERATION";
            break;
    }
    return "Undefined error";
#else
#if defined(_WIN32)
    const auto* errorString = gluErrorUnicodeStringEXT(err);
    return (errorString ? util::fromWstring(errorString) : "Undefined error");
#else
    const auto* errorString = gluErrorString(err);
    return (errorString ? std::string(reinterpret_cast<const char*>(errorString)) : "Undefined error");
#endif
#endif
}

void LogGLError(std::string_view source, std::string_view fileName, std::string_view functionName,
                int lineNumber) {
    GLuint maxErrors = 255;
    GLenum err;
    // There might be several errors, call glGetError in a loop:
    // https://www.opengl.org/sdk/docs/man2/xhtml/glGetError.xml
    while ((err = glGetError()) != GL_NO_ERROR && maxErrors--) {
        LogCentral::getPtr()->log(source, LogLevel::Error, LogAudience::Developer, fileName,
                                  functionName, lineNumber, getGLErrorString(err));
    }
}

void LogGLError(std::string_view fileName, std::string_view functionName, int lineNumber) {
    LogGLError("OpenGL", fileName, functionName, lineNumber);
}

}  // namespace inviwo