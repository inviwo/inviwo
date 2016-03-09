/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

#ifndef IVW_INVIWOOPENGL_H
#define IVW_INVIWOOPENGL_H
#include <inviwo/core/common/inviwo.h>
#include <modules/opengl/openglmoduledefine.h>

#ifdef WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
#define GLEW_NO_GLU
#endif

#include <modules/opengl/ext/glew/include/GL/glew.h> //TODO: Why is <GL/glew.h> not working

#ifdef WIN32
#include <modules/opengl/ext/glew/include/GL/wglew.h>
#else
//#include <modules/opengl/ext/glew/include/GL/glxew.h>
#endif

#include <iostream>
#include <sstream>

#include "glformats.h"

namespace inviwo {

IVW_MODULE_OPENGL_API void LogGLError(const char* fileName, const char* functionName, int lineNumber);

#if defined(IVW_DEBUG)
#define LGL_ERROR inviwo::LogGLError(__FILE__, __FUNCTION__, __LINE__)
#define LGL_ERROR_SUPPRESS glGetError()
#else
#define LGL_ERROR
#define LGL_ERROR_SUPPRESS
#endif

}

#endif // IVW_INVIWOOPENGL_H
