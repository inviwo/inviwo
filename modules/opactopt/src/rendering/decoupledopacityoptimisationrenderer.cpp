/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/opactopt/rendering/decoupledopacityoptimisationrenderer.h>

#include <modules/opengl/openglutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/volume/volumeutils.h>

namespace inviwo {

DecoupledOpacityOptimisationRenderer::DecoupledOpacityOptimisationRenderer(CameraProperty* c)
    : OpacityOptimisationRenderer(c)
    , displaydoo_{"oit/simplequad.vert", "opactopt/decoupled/display.frag", Shader::Build::No} {
    buildShaders();
}

bool DecoupledOpacityOptimisationRenderer::postPass(bool useIllustration, const Image* background) {
    // memory barrier
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

    // get query result
    GLuint numFrags = 0;
    glGetQueryObjectuiv(totalFragmentQuery_, GL_QUERY_RESULT, &numFrags);
    LGL_ERROR;

    // check if enough space was available
    if (numFrags > fragmentSize_) {
        // we have to resize the fragment storage buffer
        fragmentSize_ = static_cast<size_t>(1.1f * numFrags);

        // unbind texture
        textureUnits_.clear();
        return false;
    }

    // Build shader depending on inport state.
    if ((supportsFragmentLists() && static_cast<bool>(background) != builtWithBackground_) ||
        importanceVolumeDirty) {
        buildShaders(background);
        importanceVolumeDirty = false;
    }

    if (!useIllustration) {
        // render fragment list
        displaydoo_.activate();
        setUniforms(displaydoo_, textureUnits_[0]);
        if (builtWithBackground_) {
            // Set depth buffer to read from.
            utilgl::bindAndSetUniforms(displaydoo_, textureUnits_, *background, "bg",
                                       ImageType::ColorDepth);
        }
        displaydoo_.setUniform("reciprocalDimensions", vec2(1) / vec2(screenSize_));
        if (importanceVolume && importanceVolume->hasData())
            utilgl::bindAndSetUniforms(displaydoo_, textureUnits_, *importanceVolume);

        utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        utilgl::GlBoolState depthTest(GL_DEPTH_TEST, GL_TRUE);
        utilgl::DepthMaskState depthMask(GL_TRUE);
        utilgl::DepthFuncState depthFunc(GL_ALWAYS);
        utilgl::CullFaceState culling(GL_NONE);
        utilgl::singleDrawImagePlaneRect();
        displaydoo_.deactivate();
    }

    textureUnits_.clear();

    return true;
}

void DecoupledOpacityOptimisationRenderer::buildShaders(bool hasBackground) {
    builtWithBackground_ = hasBackground;

    auto* dfs = displaydoo_.getFragmentShaderObject();
    dfs->clearShaderExtensions();

    auto* cfs = clear_.getFragmentShaderObject();
    cfs->clearShaderExtensions();

    if (supportsFragmentLists()) {
        dfs->addShaderExtension("GL_NV_gpu_shader5", true);
        dfs->addShaderExtension("GL_EXT_shader_image_load_store", true);
        dfs->addShaderExtension("GL_NV_shader_buffer_load", true);
        dfs->addShaderExtension("GL_EXT_bindable_uniform", true);

        cfs->addShaderExtension("GL_NV_gpu_shader5", true);
        cfs->addShaderExtension("GL_EXT_shader_image_load_store", true);
        cfs->addShaderExtension("GL_NV_shader_buffer_load", true);
        cfs->addShaderExtension("GL_EXT_bindable_uniform", true);

        dfs->setShaderDefine("USE_IMPORTANCE_VOLUME",
                             importanceVolume && importanceVolume->hasData());
        dfs->setShaderDefine("BACKGROUND_AVAILABLE", builtWithBackground_);
        displaydoo_.build();
        clear_.build();
    }
}

}  // namespace inviwo
