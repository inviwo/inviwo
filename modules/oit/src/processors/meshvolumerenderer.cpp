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

#include <modules/oit/processors/meshvolumerenderer.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/image/layergl.h>
#include <inviwo/core/util/stringconversion.h>

#include <modules/oit/datastructures/rasterization.h>
#include <modules/oit/rasterizeevent.h>
#include <modules/oit/raycastingstate.h>

#include <fmt/core.h>

namespace inviwo {

/**
 * Limits the number of volume rasterizers that can be rasterized at the same time.
 * This number should also match the number of array elements in the display shader of
 * VolumeFragmentListRenderer. In particular for the volume and tf samplers as well as volume
 * parameters and the switch-case statement selecting the array samplers.
 */
constexpr int maxSupportedVolumeRasterizers = 16;

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo MeshVolumeRenderer::processorInfo_{
    "org.inviwo.MeshVolumeRenderer",  // Class identifier
    "Mesh Volume Renderer",           // Display name
    "Volume Rendering",               // Category
    CodeState::Experimental,          // Code state
    Tags::GL | Tag{"Rasterization"},  // Tags
    R"(Renderer bringing together several kinds of rasterizations objects.
       Volume rendering is performed for volume rasterization objects.
       Fragment lists are used to render the transparent pixels with correct alpha blending.)"_unindentHelp,
};
const ProcessorInfo MeshVolumeRenderer::getProcessorInfo() const { return processorInfo_; }

MeshVolumeRenderer::MeshVolumeRenderer()
    : rasterizations_("raster")
    , background_{"imageInport", "Optional background image"_help}
    , outport_("image")
    , raycastingProps_{"raycasting", "Raycasting",
                       "Properties relevant for volumetric raycasting during rasterization"_help}
    , samplingDistance_("samplingDistance", "Sampling Distance (world space)",
                        util::ordinalScale(0.01f, 1.0f)
                            .set("Distance between volume samples in world space."_help))
    , camera_("camera", "Camera")
    , lighting_{"lighting", "Lighting", &camera_}
    , trackball_(&camera_)
    , flr_{[]() -> std::optional<VolumeFragmentListRenderer> {
        if (VolumeFragmentListRenderer::supportsFragmentLists())
            return std::optional<VolumeFragmentListRenderer>{std::in_place};
        else {
            return std::nullopt;
        }
    }()} {

    if (!VolumeFragmentListRenderer::supportsFragmentLists()) {
        LogProcessorWarn(
            "Fragment lists are not supported by the hardware -> volume rasterization disabled, "
            "regular meshes rendered without sorting.");
    }

    addPort(rasterizations_);
    addPort(background_).setOptional(true);
    addPort(outport_);

    raycastingProps_.addProperties(samplingDistance_);
    addProperties(camera_, raycastingProps_, lighting_, trackball_);
    camera_.setCollapsed(true);
    trackball_.setCollapsed(true);

    if (flr_) {
        flrReload_ = flr_->onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    }

    onConnect_ = rasterizations_.onConnectScoped([&](Outport* outport) {
        RasterizeEvent e{std::dynamic_pointer_cast<MeshVolumeRenderer>(shared_from_this())};
        rasterizations_.propagateEvent(&e, outport);
    });
}

void MeshVolumeRenderer::process() {
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

    // assign volume IDs to each VolumeRasterizer
    volumeIds_.clear();
    int volumeId = 0;
    for (auto rasterization : rasterizations_) {
        if (rasterization->getRaycastingState()) {
            if (volumeId < maxSupportedVolumeRasterizers) {
                volumeIds_.emplace(rasterization->getIdentifier(), volumeId);
            }
            ++volumeId;
        }
    }
    const int numVolumes = volumeId;
    if (volumeId > maxSupportedVolumeRasterizers) {
        LogWarn(
            fmt::format("More than {} Volume Rasterizers connected to {} ({}). Omitting "
                        "surplus rasterizers.",
                        maxSupportedVolumeRasterizers, getDisplayName(), getIdentifier()));
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

        flr_->prePass(outport_.getDimensions());

        flr_->beginCount();
        for (auto rasterization : rasterizations_) {
            if (rasterization->usesFragmentLists() == UseFragmentList::Yes) {
                rasterization->rasterize(outport_.getDimensions(), mat4(1.0));
            }
        }
        flr_->endCount();

        utilgl::activateTargetAndCopySource(outport_, intermediateImage_);

        auto uniformsCallback = [&](Shader& shader, TextureUnitContainer& texUnits) {
            utilgl::setUniforms(shader, camera_, lighting_);
            shader.setUniform("samplingDistance", samplingDistance_);

            // NOTE: _All_ texture array samplers must be set (volumeSamplers[], tfSamplers[], ...).
            // Otherwise, if an unbound array sampler is used (e.g. in a switch-case statement as
            // compile-time expression), the draw call results in an invalid operation. The same
            // error is generated when setting the samplers to 0. This is currently guaranteed by
            // the VolumeFragmentListRenderer recompiling the shader for the number of available
            // volumes (numVolumes).

            StrBuffer buff;
            for (auto rasterization : rasterizations_) {
                if (auto it = volumeIds_.find(rasterization->getIdentifier());
                    it != volumeIds_.end()) {
                    const int id = it->second;
                    auto raycastingState = rasterization->getRaycastingState();

                    if (raycastingState && id < numVolumes) {
                        shader.setUniform(buff.replace("volumeChannels[{}]", id),
                                          raycastingState->channel);
                        shader.setUniform(buff.replace("opacityScaling[{}]", id),
                                          raycastingState->opacityScaling);

                        auto& volUnit = texUnits.emplace_back();
                        utilgl::bindTexture(*raycastingState->volume, volUnit);
                        shader.setUniform(buff.replace("volumeSamplers[{}]", id),
                                          volUnit.getUnitNumber());
                        utilgl::setShaderUniforms(shader, *raycastingState->volume,
                                                  StrBuffer{"volumeParameters[{}]", id});

                        if (raycastingState->tfLookup) {
                            auto& tfUnit = texUnits.emplace_back();
                            tfUnit.activate();
                            const auto* layerGL =
                                raycastingState->tfLookup->getRepresentation<LayerGL>();
                            layerGL->bindTexture(tfUnit);
                            shader.setUniform(buff.replace("tfSamplers[{}]", id),
                                              tfUnit.getUnitNumber());
                        }
                    }
                }
            }
        };

        success = flr_->postPass(false, &intermediateImage_, uniformsCallback, numVolumes);
    }

    utilgl::deactivateCurrentTarget();
}

void MeshVolumeRenderer::configureShader(Shader& shader) const {
    utilgl::addDefines(shader, lighting_);
}

void MeshVolumeRenderer::setUniforms(Shader& shader, UseFragmentList useFragmentList,
                                     std::string_view rasterizerId) const {
    utilgl::setUniforms(shader, camera_, lighting_);
    if (flr_ && useFragmentList == UseFragmentList::Yes) {
        flr_->setShaderUniforms(shader);

        if (auto it = volumeIds_.find(rasterizerId); it != volumeIds_.end()) {
            shader.setUniform("volumeId", it->second);
        }
    }
}

DispatcherHandle<void()> MeshVolumeRenderer::addInitializeShaderCallback(
    std::function<void()> callback) {
    return initializeShader_.add(callback);
}

}  // namespace inviwo
