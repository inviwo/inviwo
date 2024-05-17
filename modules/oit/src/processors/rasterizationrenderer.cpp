/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

#include <modules/oit/processors/rasterizationrenderer.h>

#include <inviwo/core/algorithm/boundingbox.h>
#include <inviwo/core/datastructures/image/image.h>        // for Image
#include <inviwo/core/interaction/cameratrackball.h>       // for CameraTrackball
#include <inviwo/core/ports/imageport.h>                   // for BaseImageInport, Ima...
#include <inviwo/core/ports/inportiterable.h>              // for InportIterable<>::co...
#include <inviwo/core/ports/outportiterable.h>             // for OutportIterable
#include <inviwo/core/processors/processor.h>              // for Processor
#include <inviwo/core/processors/processorinfo.h>          // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>         // for CodeState, CodeState...
#include <inviwo/core/processors/processortags.h>          // for Tags, Tags::GL
#include <inviwo/core/properties/boolcompositeproperty.h>  // for BoolCompositeProperty
#include <inviwo/core/properties/boolproperty.h>           // for BoolProperty
#include <inviwo/core/properties/cameraproperty.h>         // for CameraProperty
#include <inviwo/core/properties/invalidationlevel.h>      // for InvalidationLevel
#include <inviwo/core/properties/ordinalproperty.h>        // for FloatProperty, Float...
#include <inviwo/core/properties/property.h>               // for Property
#include <inviwo/core/properties/propertysemantics.h>      // for PropertySemantics
#include <inviwo/core/util/dispatcher.h>                   // for Dispatcher<>::Handle
#include <inviwo/core/util/glmmat.h>                       // for mat4
#include <inviwo/core/util/glmvec.h>                       // for uvec3, vec3
#include <inviwo/core/util/logcentral.h>                   // for LogCentral, LogProce...
#include <modules/oit/datastructures/rasterization.h>      // IWYU pragma: keep
#include <modules/oit/processors/rasterizer.h>
#include <modules/oit/ports/rasterizationport.h>         // for RasterizationInport
#include <modules/oit/rendering/fragmentlistrenderer.h>  // for FragmentListRenderer
#include <modules/opengl/shader/shaderutils.h>           // for ImageInport, setUnif...
#include <modules/opengl/texture/textureutils.h>         // for activateTargetAndCle...

#include <modules/oit/rasterizeevent.h>

#include <functional>   // for __base
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t
#include <vector>       // for vector
#include <functional>

#include <fmt/core.h>  // for format_to, basic_str...

namespace inviwo {
class Shader;

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo RasterizationRenderer::processorInfo_{
    "org.inviwo.RasterizationRenderer",  // Class identifier
    "Rasterization Renderer",            // Display name
    "Mesh Rendering",                    // Category
    CodeState::Stable,                   // Code state
    Tags::GL | Tag{"Rasterization"},     // Tags
    R"(Renderer bringing together several kinds of rasterizations objects.
       Fragment lists are used to render the transparent pixels with correct alpha blending.
       Illustration effects can be applied as a post-process.)"_unindentHelp};

const ProcessorInfo RasterizationRenderer::getProcessorInfo() const { return processorInfo_; }

void RasterizationRenderer::initializeResources() { initializeShader_.invoke(); }

RasterizationRenderer::RasterizationRenderer()
    : Processor{}
    , rasterizations_{"rastarizations",
                      "Input rasterizations filling the fragment lists/render target"_help}
    , background_{"imageInport", "Optional background image"_help}
    , outport_{"image",
               "output image containing the rendered objects and the optional input image"_help}
    , intermediateImage_{}
    , camera_{"camera", "Camera", [this]() { return boundingBox(); }}
    , lighting_{"lighting", "Lighting", &camera_}
    , trackball_{&camera_}
    , illustrationSettings_{}
    , flr_{[]() -> std::optional<FragmentListRenderer> {
        if (FragmentListRenderer::supportsFragmentLists())
            return std::optional<FragmentListRenderer>{std::in_place};
        else {
            return std::nullopt;
        }
    }()} {

    if (!FragmentListRenderer::supportsFragmentLists()) {
        LogProcessorWarn(
            "Fragment lists are not supported by the hardware -> use blending without sorting, "
            "may lead to errors");
    }
    if (!FragmentListRenderer::supportsIllustration()) {
        LogProcessorWarn(
            "Illustration Buffer not supported by the hardware, screen-space silhouettes not "
            "available");
    }

    // input and output ports
    addPort(rasterizations_);
    addPort(background_).setOptional(true);
    addPort(outport_);

    addProperties(illustrationSettings_.enabled_, lighting_, camera_, trackball_);
    camera_.setCollapsed(true);
    trackball_.setCollapsed(true);

    illustrationSettings_.enabled_.setReadOnly(!FragmentListRenderer::supportsIllustration());

    if (flr_) {
        flrReload_ = flr_->onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    }

    onConnect_ = rasterizations_.onConnectScoped([&](Outport* outport) {
        RasterizeEvent e{std::dynamic_pointer_cast<RasterizationRenderer>(shared_from_this())};
        rasterizations_.propagateEvent(&e, outport);
    });
}

std::optional<mat4> RasterizationRenderer::boundingBox() const {
    std::optional<mat4> bb;
    for (const auto& rasterization : rasterizations_) {
        bb = util::boundingBoxUnion(bb, rasterization->boundingBox());
    }
    return bb;
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

    for (auto* p : enabled_) {
        if (p != enabled_.getBoolProperty()) {
            p->readonlyDependsOn(enabled_, std::not_fn(&BoolCompositeProperty::isChecked));
        }
    }
}

FragmentListRenderer::IllustrationSettings
RasterizationRenderer::IllustrationSettings::getSettings() const {
    return FragmentListRenderer::IllustrationSettings{
        edgeColor_.get(),      edgeStrength_.get(),  haloStrength_.get(),
        smoothingSteps_.get(), edgeSmoothing_.get(), haloSmoothing_.get(),
    };
}

void RasterizationRenderer::configureShader(Shader& shader) const {
    utilgl::addDefines(shader, lighting_);
}

void RasterizationRenderer::setUniforms(Shader& shader, UseFragmentList useFragmentList,
                                        std::string_view) const {
    utilgl::setUniforms(shader, camera_, lighting_);
    if (flr_ && useFragmentList == UseFragmentList::Yes) {
        flr_->setShaderUniforms(shader);
    }
}

DispatcherHandle<void()> RasterizationRenderer::addInitializeShaderCallback(
    std::function<void()> callback) {
    return initializeShader_.add(callback);
}

void RasterizationRenderer::process() {
    if (!flr_) {
        utilgl::activateTargetAndClearOrCopySource(outport_, background_);
        for (const auto& rasterization : rasterizations_) {
            if (rasterization->usesFragmentLists() == UseFragmentList::No) {
                rasterization->rasterize(outport_.getDimensions(), mat4(1.0));
            }
        }
        utilgl::deactivateCurrentTarget();
        return;
    }

    if (intermediateImage_.getDimensions() != outport_.getDimensions()) {
        intermediateImage_.setDimensions(outport_.getDimensions());
    }

    // render into a temp image to be able to compare depths in the final pass
    utilgl::activateTargetAndClearOrCopySource(intermediateImage_, background_);
    for (const auto& rasterization : rasterizations_) {
        if (rasterization->usesFragmentLists() == UseFragmentList::No) {
            rasterization->rasterize(outport_.getDimensions(), mat4(1.0));
        }
    }

    // Loop: fragment list may need another try if not enough space for the pixels was available
    for (bool success = false; !success;) {
        // prepare fragment list rendering
        flr_->prePass(outport_.getDimensions());

        flr_->beginCount();
        for (const auto& rasterization : rasterizations_) {
            if (rasterization->usesFragmentLists() == UseFragmentList::Yes) {
                rasterization->rasterize(outport_.getDimensions(), mat4(1.0));
            }
        }
        flr_->endCount();

        utilgl::activateTargetAndCopySource(outport_, intermediateImage_);
        // final processing of fragment list rendering
        const bool useIllustration = illustrationSettings_.enabled_.isChecked() &&
                                     FragmentListRenderer::supportsIllustration();
        if (useIllustration) {
            flr_->setIllustrationSettings(illustrationSettings_.getSettings());
        }

        success = flr_->postPass(useIllustration, &intermediateImage_);
    }

    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
