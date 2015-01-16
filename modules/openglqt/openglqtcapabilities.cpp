/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#include "openglqtcapabilities.h"
#include <modules/opengl/inviwoopengl.h>
#include <inviwo/core/util/formatconversion.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/stringconversion.h>

namespace inviwo {

#define OpenGLQtInfoNotFound(message) { LogInfo(message << " Info could not be retrieved"); }

OpenGLQtCapabilities::OpenGLQtCapabilities() {}

OpenGLQtCapabilities::~OpenGLQtCapabilities() {}

void OpenGLQtCapabilities::printInfo() {
    //Qt General Info
}

std::vector<int> OpenGLQtCapabilities::getGLVersion() {
    const GLubyte* glversion = glGetString(GL_VERSION);
    std::string glVersionStr = std::string((glversion!=NULL ? reinterpret_cast<const char*>(glversion) : "INVALID"));
    std::vector<std::string> versionInfoStr = splitString(glVersionStr,'.');
    //ivwAssert(versionInfo.size()!=0, "Cannot retrieve GL version.");
    std::string majorVersion = versionInfoStr[0];
    std::string minorVersion = versionInfoStr[1];
    int major = stringTo<int>(majorVersion);
    int minor = stringTo<int>(minorVersion);
    std::vector<int> versionInfo;
    versionInfo.push_back(major);
    versionInfo.push_back(minor);
    return versionInfo;
}

} // namespace
