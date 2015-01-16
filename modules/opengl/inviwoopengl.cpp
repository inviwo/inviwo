/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

void LogGLError(const char* fileName, const char* functionName, int lineNumber) {
    GLenum err = glGetError();

    if (err != GL_NO_ERROR) {
        std::ostringstream errorMessage;
#ifdef GLEW_NO_GLU
        std::string errorString = "";
        switch (err) {
            case GL_INVALID_OPERATION:
                errorString = "INVALID_OPERATION";
                break;
            case GL_INVALID_ENUM:
                errorString = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                errorString = "INVALID_VALUE";
                break;
            case GL_OUT_OF_MEMORY:
                errorString = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                errorString = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
        errorMessage << (!errorString.empty() ? errorString.c_str() : "undefined");
#else
        const GLubyte* errorString = gluErrorString(err);
        errorMessage << (errorString ? (const char*)errorString : "undefined");
#endif
        inviwo::LogCentral::getPtr()->log("OpenGL", inviwo::Error, fileName, functionName,
                                            lineNumber, errorMessage.str());
    }
}