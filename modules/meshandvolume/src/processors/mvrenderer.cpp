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

#include <inviwo/meshandvolume/processors/mvrenderer.h>
#include <modules/meshrenderinggl/datastructures/rasterization.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo MVRenderer::processorInfo_{
    "org.inviwo.MVRenderer",     // Class identifier
    "Mesh and Volume Renderer",  // Display name
    "Volume Rendering",          // Category
    CodeState::Experimental,     // Code state
    Tags::GL,                    // Tags
};
const ProcessorInfo MVRenderer::getProcessorInfo() const { return processorInfo_; }

MVRenderer::MVRenderer()
    : Processor()
    , rasterizations_("raster")
    , imageInport_(std::make_shared<ImageInport>("imageInport"))
    , imageOutport_("image")
    , intermediateImage_()
    , camera_("camera", "Camera")
    , trackball_(&camera_)
    , flr_{[this]() {
        if (!MyFragmentListRenderer::supportsFragmentLists()) {
            throw Exception{IVW_CONTEXT, "Fragment list not supported."};
        }
        return std::make_unique<MyFragmentListRenderer>();
    }()} {

    addPort(rasterizations_);
    addPort(*imageInport_).setOptional(true);
    addPort(imageOutport_);

    addProperties(camera_, trackball_);
    camera_.setCollapsed(true);
    trackball_.setCollapsed(true);

    flrReload_ = flr_->onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    imageInport_->onChange([this]() {
        if (imageInport_->hasData()) {
            intermediateImage_.setDimensions(imageInport_->getData()->getDimensions());
        } else {
            intermediateImage_.setDimensions(imageOutport_.getData()->getDimensions());
        }
    });
}

void MVRenderer::process() {
    LGL_ERROR;

    bool fragmentLists = false;
    bool containsOpaque = false;
    for (auto rasterization : rasterizations_) {
        if (rasterization->usesFragmentLists()) {
            fragmentLists = true;
        } else {
            containsOpaque = true;
        }
    }

    utilgl::activateTargetAndClearOrCopySource(intermediateImage_, *imageInport_);

    bool useIntermediateTarget = imageInport_->getData() && fragmentLists && containsOpaque;
    if (useIntermediateTarget) {
        utilgl::activateTargetAndClearOrCopySource(intermediateImage_, *imageInport_);
    } else {
        utilgl::activateTargetAndClearOrCopySource(imageOutport_, *imageInport_);
    }

    // Loop: fragment list may need another try if not enough space for the pixels was available
    bool retry = false;
    do {
        retry = false;
        LGL_ERROR;
        if (flr_ && fragmentLists) {
            flr_->prePass(imageOutport_.getDimensions());
        }

        int volumeId = 0;
        for (auto rasterization : rasterizations_) {  // Calculate color
            rasterization->rasterize(imageOutport_.getDimensions(), mat4(1.0),
                                     [this, volumeId, fragmentLists](Shader& sh) {  // volume
                                         utilgl::setUniforms(sh, camera_);
                                         sh.setUniform("volumeId", volumeId);
                                         if (flr_ && fragmentLists) {
                                             flr_->setShaderUniforms(sh);
                                         }
                                     });
            volumeId++;
        }
        LGL_ERROR;

        if (flr_ && fragmentLists) {
            // final processing of fragment list rendering
            if (useIntermediateTarget) {
                utilgl::deactivateCurrentTarget();
                utilgl::activateTargetAndCopySource(imageOutport_, intermediateImage_);
            }

            volumeId = 0;
            TextureUnitContainer units;
            for (auto rasterization : rasterizations_) {
                if (auto raycastingState = rasterization->getRaycastingState()) {
                    flr_->setRaycastingState(raycastingState, volumeId, units);
                    // ENSURE ID < 8
                }
                ++volumeId;
            }
            LGL_ERROR;

            const Image* background =
                useIntermediateTarget ? &intermediateImage_ : imageInport_->getData().get();
            retry = !flr_->postPass(false, background);
        }
    } while (retry);

    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
