/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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
#include <inviwo/core/rendering/meshdrawer.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/buffer/bufferobject.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/util/dispatcher.h>
#include <modules/meshrenderinggl/datastructures/rasterization.h>

#include <modules/opengl/texture/textureunit.h>
#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/datastructures/light/lightingstate.h>

/*
TODO: Upload volume to shader oit/display.frag and render volume
*/

namespace inviwo {

/**
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS_FROM_A_DEVELOPER_PERSPECTIVE
 */
class IVW_MODULE_MESHANDVOLUME_API MyFragmentListRenderer {
public:
    MyFragmentListRenderer();
    ~MyFragmentListRenderer();

    void prePass(const size2_t& screenSize);

    void setShaderUniforms(Shader& shader);

    bool postPass(bool useIllustration, const Image* background);

    static bool supportsFragmentLists();

    typename Dispatcher<void()>::Handle onReload(std::function<void()> callback);
    void setRaycastingState(const Rasterization::RaycastingState* rp, int id, TextureUnitContainer& units);

private:
    void buildShaders(bool hasBackground = false);

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
