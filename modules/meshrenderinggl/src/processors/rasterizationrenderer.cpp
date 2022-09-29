/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
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

#include <modules/meshrenderinggl/processors/rasterizationrenderer.h>

#include <inviwo/core/datastructures/image/image.h>                  // for Image
#include <inviwo/core/interaction/cameratrackball.h>                 // for CameraTrackball
#include <inviwo/core/ports/imageport.h>                             // for BaseImageInport, Ima...
#include <inviwo/core/ports/inportiterable.h>                        // for InportIterable<>::co...
#include <inviwo/core/ports/outportiterable.h>                       // for OutportIterable
#include <inviwo/core/processors/processor.h>                        // for Processor
#include <inviwo/core/processors/processorinfo.h>                    // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                   // for CodeState, CodeState...
#include <inviwo/core/processors/processortags.h>                    // for Tags, Tags::GL
#include <inviwo/core/properties/boolcompositeproperty.h>            // for BoolCompositeProperty
#include <inviwo/core/properties/boolproperty.h>                     // for BoolProperty
#include <inviwo/core/properties/cameraproperty.h>                   // for CameraProperty
#include <inviwo/core/properties/invalidationlevel.h>                // for InvalidationLevel
#include <inviwo/core/properties/ordinalproperty.h>                  // for FloatProperty, Float...
#include <inviwo/core/properties/property.h>                         // for Property
#include <inviwo/core/properties/propertysemantics.h>                // for PropertySemantics
#include <inviwo/core/util/dispatcher.h>                             // for Dispatcher<>::Handle
#include <inviwo/core/util/glmmat.h>                                 // for mat4
#include <inviwo/core/util/glmvec.h>                                 // for uvec3, vec3
#include <inviwo/core/util/logcentral.h>                             // for LogCentral, LogProce...
#include <modules/meshrenderinggl/datastructures/rasterization.h>    // IWYU pragma: keep
#include <modules/meshrenderinggl/ports/rasterizationport.h>         // for RasterizationInport
#include <modules/meshrenderinggl/rendering/fragmentlistrenderer.h>  // for FragmentListRenderer
#include <modules/opengl/shader/shaderutils.h>                       // for ImageInport, setUnif...
#include <modules/opengl/texture/textureutils.h>                     // for activateTargetAndCle...

#include <functional>                                                // for __base
#include <string>                                                    // for string
#include <string_view>                                               // for string_view
#include <type_traits>                                               // for remove_extent_t
#include <vector>                                                    // for vector

#include <fmt/core.h>                                                // for format_to, basic_str...

namespace inviwo {
class Shader;

namespace {
void configComposite(BoolCompositeProperty& comp) {

    auto callback = [&comp]() mutable {
        comp.setCollapsed(!comp.isChecked());
        for (auto p : comp) {
            if (p == comp.getBoolProperty()) continue;
            p->setReadOnly(!comp.isChecked());
        }
    };

    comp.getBoolProperty()->onChange(callback);
    callback();
}
}  // namespace

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo RasterizationRenderer::processorInfo_{
    "org.inviwo.RasterizationRenderer",  // Class identifier
    "Rasterization Renderer",            // Display name
    "Mesh Rendering",                    // Category
    CodeState::Stable,                   // Code state
    Tags::GL,                            // Tags
};
const ProcessorInfo RasterizationRenderer::getProcessorInfo() const { return processorInfo_; }

RasterizationRenderer::RasterizationRenderer()
    : Processor()
    , rasterizations_("rastarizations")
    , imageInport_(std::make_shared<ImageInport>("imageInport"))
    , outport_("image")
    , intermediateImage_()
    , camera_("camera", "Camera")
    , trackball_(&camera_)
    , flr_{FragmentListRenderer::supportsFragmentLists() ? std::make_unique<FragmentListRenderer>()
                                                         : nullptr}
    , supportesIllustration_{FragmentListRenderer::supportsIllustration()} {

    if (!FragmentListRenderer::supportsFragmentLists()) {
        LogProcessorWarn(
            "Fragment lists are not supported by the hardware -> use blending without sorting, may "
            "lead to errors");
    }
    if (!supportesIllustration_) {
        LogProcessorWarn(
            "Illustration Buffer not supported by the hardware, screen-space silhouettes not "
            "available");
    }

    // input and output ports
    addPort(rasterizations_);
    addPort(*imageInport_).setOptional(true);
    addPort(outport_);

    addProperties(camera_, trackball_, illustrationSettings_.enabled_);
    camera_.setCollapsed(true);
    trackball_.setCollapsed(true);

    illustrationSettings_.enabled_.setReadOnly(!supportesIllustration_);

    if (flr_) {
        flrReload_ = flr_->onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    }

    imageInport_->onChange([this]() {
        if (imageInport_->hasData()) {
            intermediateImage_.setDimensions(imageInport_->getData()->getDimensions());
        } else {
            intermediateImage_.setDimensions(outport_.getData()->getDimensions());
        }
    });
}

RasterizationRenderer::IllustrationSettings::IllustrationSettings()
    : enabled_("illustration", "Use Illustration Effects", false)
    , edgeColor_("edgeColor", "Edge Color", vec3(0.0f))
    , edgeStrength_("edgeStrength", "Edge Strength", 0.5f, 0.f, 1.f, 0.01f)
    , haloStrength_("haloStrength", "Halo Strength", 0.5f, 0.f, 1.f, 0.01f)
    , smoothingSteps_("smoothingSteps", "Smoothing Steps", 3, 0, 50, 1)
    , edgeSmoothing_("edgeSmoothing", "Edge Smoothing", 0.8f, 0.f, 1.f, 0.01f)
    , haloSmoothing_("haloSmoothing", "Halo Smoothing", 0.8f, 0.f, 1.f, 0.01f) {

    edgeColor_.setSemantics(PropertySemantics::Color);
    enabled_.addProperties(edgeColor_, edgeStrength_, haloStrength_, smoothingSteps_,
                           edgeSmoothing_, haloSmoothing_);

    configComposite(enabled_);
}

FragmentListRenderer::IllustrationSettings
RasterizationRenderer::IllustrationSettings::getSettings() const {
    return FragmentListRenderer::IllustrationSettings{
        edgeColor_.get(),      edgeStrength_.get(),  haloStrength_.get(),
        smoothingSteps_.get(), edgeSmoothing_.get(), haloSmoothing_.get(),
    };
}

void RasterizationRenderer::process() {
    bool fragmentLists = false;
    bool containsOpaque = false;
    for (auto rasterization : rasterizations_) {
        if (rasterization->usesFragmentLists()) {
            fragmentLists = true;
        } else {
            containsOpaque = true;
        }
    }

    bool useIntermediateTarget = imageInport_->getData() && fragmentLists && containsOpaque;
    if (useIntermediateTarget) {
        utilgl::activateTargetAndClearOrCopySource(intermediateImage_, *imageInport_);
    } else {
        utilgl::activateTargetAndClearOrCopySource(outport_, *imageInport_);
    }

    // Loop: fragment list may need another try if not enough space for the pixels was available
    bool retry = false;
    do {
        retry = false;

        if (flr_ && fragmentLists) {
            // prepare fragment list rendering
            flr_->prePass(outport_.getDimensions());
        }

        for (auto rasterization : rasterizations_) {
            rasterization->rasterize(outport_.getDimensions(), mat4(1.0),
                                     [this, fragmentLists](Shader& sh) {
                                         utilgl::setUniforms(sh, camera_);
                                         if (flr_ && fragmentLists) flr_->setShaderUniforms(sh);
                                     });
        }

        if (flr_ && fragmentLists) {
            // final processing of fragment list rendering
            if (useIntermediateTarget) {
                utilgl::deactivateCurrentTarget();
                utilgl::activateTargetAndCopySource(outport_, intermediateImage_);
            }

            const bool useIllustration =
                illustrationSettings_.enabled_.isChecked() && supportesIllustration_;
            if (useIllustration) {
                flr_->setIllustrationSettings(illustrationSettings_.getSettings());
            }
            const Image* background =
                useIntermediateTarget ? &intermediateImage_ : imageInport_->getData().get();
            retry = !flr_->postPass(useIllustration, background);
        }
    } while (retry);

    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
