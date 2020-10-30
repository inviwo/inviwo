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

#pragma once

#include <modules/opengl/openglmoduledefine.h>

#ifdef __APPLE__
#define GLEW_NO_GLU
#endif

#include <GL/glew.h>

#include <string_view>
#include <typeinfo>

namespace inviwo {

/** \brief Returns a readable interpretation of the OpenGL error.
 *
 * @param err OpenGL error enum, GLenum err = glGetError();
 * @return Returns "No error" if err == GL_NO_ERROR, otherwise the name of the error.
 */
IVW_MODULE_OPENGL_API std::string getGLErrorString(GLenum err);

/**
 * Log the last OpenGL error if there has been an error, i.e. glGetError() != GL_NO_ERROR.
 */
IVW_MODULE_OPENGL_API void LogGLError(std::string_view fileName, std::string_view functionName,
                                      int lineNumber);

/**
 * Log the last OpenGL error if there has been an error, i.e. glGetError() != GL_NO_ERROR.
 */
IVW_MODULE_OPENGL_API void LogGLError(std::string_view source, std::string_view fileName,
                                      std::string_view functionName, int lineNumber);

IVW_MODULE_OPENGL_API void LogGLError(const std::type_info& source, std::string_view fileName,
                                      std::string_view functionName, int lineNumber);

#if defined(IVW_DEBUG) || defined(IVW_FORCE_ASSERTIONS)
#define LGL_ERROR inviwo::LogGLError(__FILE__, __FUNCTION__, __LINE__)
#define LGL_ERROR_CLASS inviwo::LogGLError(typeid(this), __FILE__, __FUNCTION__, __LINE__)
#define LGL_ERROR_SUPPRESS glGetError()
#else
#define LGL_ERROR
#define LGL_ERROR_CLASS
#define LGL_ERROR_SUPPRESS
#endif

}  // namespace inviwo
