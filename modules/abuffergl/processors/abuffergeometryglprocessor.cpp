/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <modules/abuffergl/processors/abuffergeometryglprocessor.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/image/imagegl.h>
#include <inviwo/core/rendering/meshdrawerfactory.h>
#include <modules/opengl/image/layergl.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <modules/opengl/image/imagegl.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/util/filesystem.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/openglutils.h>

#include <fstream>

namespace inviwo {

const ProcessorInfo ABufferGeometryGLProcessor::processorInfo_{
    "org.inviwo.ABufferGeometryGLProcessor",  // Class identifier
    "ABufferGeometryGLProcessor",             // Display name
    "ABufferDemo",                            // Category
    CodeState::Experimental,                  // Code state
    Tags::None,                               // Tags
};
const ProcessorInfo ABufferGeometryGLProcessor::getProcessorInfo() const {
    return processorInfo_;
}

ABufferGeometryGLProcessor::ABufferGeometryGLProcessor()
    : MeshRenderProcessorGL()
    , abuffer_()
    , transparency_("transparency", "Global Transparency", 0.5f, 0.0f, 1.0f, 0.001f)
    , abufferGeometryShader_("geometryrendering.vert", "abuffergeometrygl.frag", false) {
    addProperty(abuffer_.settings_);
    addProperty(transparency_);

    abuffer_.settings_.onChange(this, &ABufferGeometryGLProcessor::onAbufferSettingChanged);
    transparency_.onChange(this, &ABufferGeometryGLProcessor::onAbufferTransparencyChanged);
}

ABufferGeometryGLProcessor::~ABufferGeometryGLProcessor() {}

void ABufferGeometryGLProcessor::initializeResources() {
    MeshRenderProcessorGL::initializeResources();
    MeshRenderProcessorGL::addCommonShaderDefines(abufferGeometryShader_);
}

void ABufferGeometryGLProcessor::process() { geometryRender(); }

void ABufferGeometryGLProcessor::geometryRender() {
    ABUFFER_PROFILE("Total-Time");

    // tempImage_ = new Image(outport_.getDimensions());
    auto tempImage_ = outport_.getEditableData();
    ImageGL* tempImageGL = tempImage_->getEditableRepresentation<ImageGL>();
    ivec2 dim = outport_.getDimensions();

    bool abufferEnabled = abuffer_.settings_.abufferEnable_.get();

    // abuffer initialization
    if (abufferEnabled) {
        ABUFFER_PROFILE("Initialize-abuffer");
        abuffer_.abuffer_initABuffer(dim, updateRequried_);

        LGL_ERROR;
        abuffer_.abuffer_addShaderDefinesAndBuild(&abuffer_.resolveABufferShader_);

        LGL_ERROR;
        ABUFFER_PROFILE("Reset-abuffer");
        abuffer_.aBuffer_resetLinkList(tempImageGL);

        updateRequried_ = false;
    }

    abuffer_.abuffer_addShaderDefinesAndBuild(&abufferGeometryShader_);

    // Rendering
    {
        ABUFFER_PROFILE("Rendering");

        tempImageGL = tempImage_->getEditableRepresentation<ImageGL>();
        utilgl::activateAndClearTarget(outport_);

        // initialize shader
        abufferGeometryShader_.activate();
        // LGL_ERROR;
        utilgl::setShaderUniforms(abufferGeometryShader_, camera_, "camera_");
        // LGL_ERROR;
        utilgl::setShaderUniforms(abufferGeometryShader_, lightingProperty_, "light_");
        // LGL_ERROR;

        if (abufferEnabled) {
            abuffer_.abuffer_addUniforms(&abufferGeometryShader_);
            abufferGeometryShader_.setUniform("globalTransparency_", transparency_.get());
        }

        utilgl::GlBoolState depthTest(GL_DEPTH_TEST, true);
        utilgl::CullFaceState culling(cullFace_.get());
        utilgl::PolygonModeState polygon(polygonMode_.get(), renderLineWidth_, renderPointSize_);

        for (auto& drawer : drawers_) {
            utilgl::setShaderUniforms(abufferGeometryShader_, *(drawer.second->getGeometry()),
                                      "geometry_");
            drawer.second->draw();
        }

        abufferGeometryShader_.deactivate();
        utilgl::deactivateCurrentTarget();

        glMemoryBarrierEXT(GL_ALL_BARRIER_BITS_EXT);
    }

    // abuffer resolve
    if (abufferEnabled) {
        ABUFFER_PROFILE("Resolving-abuffer");
        utilgl::activateAndClearTarget(outport_);
        abuffer_.aBuffer_resolveLinkList(tempImageGL, imageInport_.getData().get());
        utilgl::deactivateCurrentTarget();
    }
}

void ABufferGeometryGLProcessor::onAbufferSettingChanged() { updateRequried_ = true; }

void ABufferGeometryGLProcessor::onAbufferTransparencyChanged() { /*updateRequried_ = true;*/ }

}  // namespace

