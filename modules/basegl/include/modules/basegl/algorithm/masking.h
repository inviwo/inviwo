/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>

#include <inviwo/core/datastructures/volume/volume.h>

#include <modules/opengl/shader/shader.h>
#include <modules/opengl/buffer/framebufferobject.h>

#include <string>

namespace inviwo {
class StringShaderResource;

namespace mask {

struct IVW_MODULE_BASEGL_API Config {
    bool maskMissingValue = false;
    bool maskZero = false;
    bool maskNaN = false;
    bool maskInf = false;

    constexpr bool operator==(const Config&) const = default;
};

IVW_MODULE_BASEGL_API std::string makeFragmentShader(Config config);

struct IVW_MODULE_BASEGL_API MaskTool {
    MaskTool();
    std::shared_ptr<Volume> process(const Config& config, const Volume& volume,
                                    std::shared_ptr<Volume> reuse = nullptr);

    Config current;
    std::shared_ptr<StringShaderResource> frag;
    Shader shader;
    FrameBufferObject fbo;
};

}  // namespace mask
}  // namespace inviwo
