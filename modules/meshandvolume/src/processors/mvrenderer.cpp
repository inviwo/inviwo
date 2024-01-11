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

#include <inviwo/meshandvolume/processors/mvrenderer.h>
#include <modules/meshrenderinggl/datastructures/rasterization.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <inviwo/core/util/stringconversion.h>

#include <fmt/core.h>

namespace inviwo {

/**
 * Limits the number of volume rasterizers that can be rasterized at the same time.
 * This number should also match the number of array elements in the display shader of
 * MyFragmentListRenderer. In particular for the volume and tf samplers as well as volume
 * parameters and the switch-case statement selecting the array samplers.
 */
constexpr int maxSupportedVolumeRasterizers = 4;

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
    : rasterizations_("raster")
    , imageInport_(std::make_shared<ImageInport>("imageInport"))
    , imageOutport_("image")
    , raycastingProps_{"raycasting", "Raycasting",
                       "Properties relevant for volumetric raycasting during rasterization"_help}
    , samplingDistance_("samplingDistance", "Sampling Distance (world space)",
                        util::ordinalScale(0.01f, 1.0f)
                            .set("Distance between volume samples in world space."_help))
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

    addProperties(camera_, raycastingProps_);
    raycastingProps_.addProperties(samplingDistance_, trackball_);
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
                                     [this, &volumeId, fragmentLists](Shader& sh) {  // volume
                                         utilgl::setUniforms(sh, camera_);
                                         sh.setUniform("volumeId", volumeId);
                                         if (flr_ && fragmentLists) {
                                             flr_->setShaderUniforms(sh);
                                         }
                                     });
            if (rasterization->getRaycastingState()) {
                ++volumeId;
            }
            LGL_ERROR;
        }
        LGL_ERROR;
        if (volumeId >= maxSupportedVolumeRasterizers) {
            LogWarn(
                fmt::format("More than {} Volume Rasterizers connected to {} ({}). Omitting "
                            "surplus rasterizers.",
                            maxSupportedVolumeRasterizers, getDisplayName(), getIdentifier()));
        }

        if (flr_ && fragmentLists) {
            // final processing of fragment list rendering
            if (useIntermediateTarget) {
                utilgl::deactivateCurrentTarget();
                utilgl::activateTargetAndCopySource(imageOutport_, intermediateImage_);
            }

            auto uniformsCallback = [&](Shader& shader) {
                utilgl::setShaderUniforms(shader, camera_, "camera");
                shader.setUniform("samplingDistance", samplingDistance_);

                StrBuffer buff;
                TextureUnitContainer texUnits;
                int volumeId = 0;
                for (auto rasterization : rasterizations_) {
                    auto raycastingState = rasterization->getRaycastingState();
                    if (raycastingState && volumeId < maxSupportedVolumeRasterizers) {
                        shader.setUniform(buff.replace("volumeChannels[{}]", volumeId),
                                          raycastingState->channel);
                        shader.setUniform(buff.replace("opacityScaling[{}]", volumeId),
                                          raycastingState->opacityScaling);

                        auto& volUnit = texUnits.emplace_back();
                        utilgl::bindTexture(*raycastingState->volume, volUnit);
                        shader.setUniform(buff.replace("volumeSamplers[{}]", volumeId),
                                          volUnit.getUnitNumber());
                        utilgl::setShaderUniforms(shader, *raycastingState->volume,
                                                  StrBuffer{"volumeParameters[{}]", volumeId});

                        auto& tfUnit = texUnits.emplace_back();
                        utilgl::bindTexture(raycastingState->tf, tfUnit);
                        shader.setUniform(buff.replace("tfSamplers[{}]", volumeId),
                                          tfUnit.getUnitNumber());

                        utilgl::setShaderUniforms(shader, raycastingState->lighting,
                                                  buff.replace("lighting[{}]", volumeId));

                        ++volumeId;
                    }
                }
                LGL_ERROR;

                // Duplicate volume and TF texture unit IDs for remaining unused texture array
                // samplers (up to maxSupportedVolumeRasterizers). Otherwise, if an unbound array
                // sampler is used (e.g. in the switch-case statement as compile-time expression),
                // the draw call results in an invalid operation. The same error is generated when
                // setting the samplers to 0.
                const GLint volumeSampler = texUnits[texUnits.size() - 2].getUnitNumber();
                const GLint tfSampler = texUnits[texUnits.size() - 1].getUnitNumber();
                while (volumeId > 0 && volumeId < maxSupportedVolumeRasterizers) {
                    shader.setUniform(buff.replace("volumeSamplers[{}]", volumeId), volumeSampler);
                    shader.setUniform(buff.replace("tfSamplers[{}]", volumeId), tfSampler);
                    ++volumeId;
                }
                LGL_ERROR;
            };

            const Image* background =
                useIntermediateTarget ? &intermediateImage_ : imageInport_->getData().get();
            retry = !flr_->postPass(false, background, uniformsCallback);
        }
        LGL_ERROR;
    } while (retry);
    LGL_ERROR;

    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
