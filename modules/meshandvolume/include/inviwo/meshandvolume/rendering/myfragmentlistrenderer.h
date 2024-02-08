/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2024 Inviwo Foundation
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

#include <inviwo/meshandvolume/meshandvolumemoduledefine.h>

#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/buffer/bufferobject.h>
#include <inviwo/core/util/dispatcher.h>
#include <modules/meshrenderinggl/datastructures/rasterization.h>

#include <modules/opengl/texture/textureunit.h>
#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/datastructures/light/lightingstate.h>

namespace inviwo {

class Image;

class IVW_MODULE_MESHANDVOLUME_API MyFragmentListRenderer {
public:
    MyFragmentListRenderer();
    ~MyFragmentListRenderer();

    void prePass(const size2_t& screenSize);

    void setShaderUniforms(Shader& shader);

    bool postPass(bool useIllustration, const Image* background,
                  std::function<void(Shader&)> setUniformsCallback);

    static bool supportsFragmentLists();

    typename Dispatcher<void()>::Handle onReload(std::function<void()> callback);

private:
    void initializeClearShader();
    void initializeDisplayShader();
    void rebuildShaders(bool hasBackground = false);

    void setUniforms(Shader& shader, TextureUnit& abuffUnit) const;
    void resizeBuffers(const size2_t& screenSize);

    size2_t screenSize_;
    size_t fragmentSize_;

    // basic fragment lists
    Texture2D abufferIdxTex_;
    TextureUnitContainer textureUnits_;
    bool builtWithBackground_ = false;

    BufferObject atomicCounter_;
    BufferObject pixelBuffer_;

    GLuint totalFragmentQuery_;

    Shader clear_;
    Shader display_;

    Dispatcher<void()> onReload_;
};

}  // namespace inviwo
