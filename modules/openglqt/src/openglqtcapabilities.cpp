/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <modules/openglqt/openglqtcapabilities.h>

#include <inviwo/core/util/stringconversion.h>  // for stringTo, splitStringView
#include <modules/opengl/inviwoopengl.h>        // for glGetString, GL_VERSION, GLubyte

#include <string>  // for string

namespace inviwo {

OpenGLQtCapabilities::OpenGLQtCapabilities() = default;

OpenGLQtCapabilities::~OpenGLQtCapabilities() = default;

void OpenGLQtCapabilities::printInfo() {
    // Qt General Info
}

std::vector<int> OpenGLQtCapabilities::getGLVersion() {
    const GLubyte* glversion = glGetString(GL_VERSION);
    std::string glVersionStr =
        std::string((glversion != nullptr ? reinterpret_cast<const char*>(glversion) : "INVALID"));
    const auto versionInfoStr = util::splitStringView(glVersionStr, '.');
    const int major = stringTo<int>(versionInfoStr[0]);
    const int minor = stringTo<int>(versionInfoStr[1]);
    return {major, minor};
}

}  // namespace inviwo
