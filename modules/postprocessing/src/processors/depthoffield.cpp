/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2023 Inviwo Foundation
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

#include <modules/postprocessing/processors/depthoffield.h>

#include <inviwo/core/common/inviwoapplication.h>                       // for dispatchFront
#include <inviwo/core/datastructures/buffer/buffer.h>                   // for BufferBase
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAM
#include <inviwo/core/datastructures/camera/camera.h>                   // for Camera
#include <inviwo/core/datastructures/camera/perspectivecamera.h>        // for PerspectiveCamera
#include <inviwo/core/datastructures/camera/skewedperspectivecamera.h>  // for SkewedPerspective...
#include <inviwo/core/datastructures/image/image.h>                     // for Image
#include <inviwo/core/datastructures/image/imageram.h>                  // for ImageRAM
#include <inviwo/core/datastructures/image/imagetypes.h>                // for ImageType, ImageT...
#include <inviwo/core/datastructures/image/layer.h>                     // for Layer
#include <inviwo/core/datastructures/image/layerram.h>                  // for LayerRAM
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume
#include <inviwo/core/datastructures/volume/volumeram.h>                // for VolumeRAM
#include <inviwo/core/interaction/events/keyboardkeys.h>                // for KeyModifier, KeyM...
#include <inviwo/core/interaction/events/mousebuttons.h>                // for MouseButton, Mous...
#include <inviwo/core/interaction/events/mouseevent.h>                  // for MouseEvent
#include <inviwo/core/network/networklock.h>                            // for NetworkLock
#include <inviwo/core/ports/imageport.h>                                // for BaseImageInport
#include <inviwo/core/ports/meshport.h>                                 // for MeshInport
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags, Tags::None
#include <inviwo/core/properties/boolproperty.h>                        // for BoolProperty
#include <inviwo/core/properties/cameraproperty.h>                      // for CameraProperty
#include <inviwo/core/properties/eventproperty.h>                       // for EventProperty
#include <inviwo/core/properties/invalidationlevel.h>                   // for InvalidationLevel
#include <inviwo/core/properties/ordinalproperty.h>                     // for IntSizeTProperty
#include <inviwo/core/util/formats.h>                                   // for DataFormat
#include <inviwo/core/util/glmvec.h>                                    // for vec2, size2_t, vec3
#include <inviwo/core/util/logcentral.h>                                // for LogCentral, LogWarn
#include <modules/base/algorithm/randomutils.h>                         // for haltonSequence
#include <modules/opengl/image/imagegl.h>                               // for ImageGL
#include <modules/opengl/inviwoopengl.h>                                // for GLuint, GL_FALSE
#include <modules/opengl/openglcapabilities.h>                          // for OpenGLCapabilities
#include <modules/opengl/shader/shader.h>                               // for Shader, Shader::B...
#include <modules/opengl/shader/shadertype.h>                           // for ShaderType, Shade...
#include <modules/opengl/shader/shaderutils.h>                          // for ImageInport
#include <modules/opengl/texture/textureunit.h>                         // for TextureUnitContainer
#include <modules/opengl/texture/textureutils.h>                        // for bindAndSetUniforms
#include <modules/opengl/volume/volumegl.h>                             // for VolumeGL
#include <modules/opengl/volume/volumeutils.h>                          // for bindAndSetUniforms
#include <modules/opengl/texture/texture3d.h>                           // IWYU pragma: keep

#include <algorithm>      // for max, min, copy, fill
#include <cmath>          // for cos, sin, pow, round
#include <functional>     // for __base
#include <numeric>        // for accumulate
#include <ostream>        // for operator<<
#include <string>         // for string, operator!=
#include <string_view>    // for string_view
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set
#include <utility>        // for pair, make_pair
#include <numbers>

#include <glm/detail/setup.hpp>               // for size_t
#include <glm/fwd.hpp>                        // for vec2, vec3
#include <glm/geometric.hpp>                  // for normalize, dot
#include <glm/gtx/scalar_multiplication.hpp>  // for operator*, operator/
#include <glm/trigonometric.hpp>              // for cos, radians, sin
#include <glm/vec2.hpp>                       // for vec<>::(anonymous)
#include <glm/vec3.hpp>                       // for operator*, vec<>:...

namespace inviwo {
class Event;

const ProcessorInfo DepthOfField::processorInfo_{
    "org.inviwo.DepthOfField",  // Class identifier
    "Depth Of Field",           // Display name
    "Postprocessing",           // Category
    CodeState::Experimental,    // Code state
    Tags::None,                 // Tags
};
const ProcessorInfo DepthOfField::getProcessorInfo() const { return processorInfo_; }

DepthOfField::DepthOfField()
    : Processor()
    , inport_("inport")
    , trackingInport_("trackingInport")
    , outport_("outport")
    , aperture_("aperture", "Aperture", 0.5, 0.01, 1., 0.01)
    , focusDepth_("focusDepth", "Focus depth", 8., 1.0, 15.0, 0.1)
    , manualFocus_("manualFocus", "Manual focus", true)
    , approximate_("approximate", "Approximate", false)
    , viewCountExact_("viewCountExact", "View count", 40, 10, 200)
    , viewCountApprox_("viewCountApprox", "Rendered view count", 5, 1, 12)
    , simViewCountApprox_("simViewCountApprox", "Simulated view count", 40, 10, 200)
    , clickToFocus_(
          "clickToFocus", "Click to focus",
          [this](Event* e) {
              if (manualFocus_) clickToFocus(e);
          },
          MouseButton::Left, MouseState::Press, KeyModifier::Control)
    , camera_("camera", "Camera")
    , evalCount_(-1)
    , useComputeShaders_(OpenGLCapabilities::getOpenGLVersion() >= 430)
    , ogCamera_{}
    , addSampleShader_("fullscreenquad.vert", "dof_exact.frag")
    , addToLightFieldShader_({{ShaderType::Compute, "dof_approx.comp"}}, Shader::Build::No)
    , averageLightFieldShader_("fullscreenquad.vert", "dof_approx.frag") {

    addPort(inport_);
    addPort(trackingInport_);
    addPort(outport_);
    addProperties(aperture_, focusDepth_, manualFocus_, approximate_, viewCountExact_,
                  viewCountApprox_, simViewCountApprox_, clickToFocus_, camera_);

    focusDepth_.readonlyDependsOn(manualFocus_, [](auto l) { return !l; });
    clickToFocus_.readonlyDependsOn(manualFocus_, [](auto l) { return !l; });
    viewCountExact_.visibilityDependsOn(approximate_, [](auto l) { return !l; });
    viewCountApprox_.visibilityDependsOn(approximate_, [](auto l) { return l; });
    simViewCountApprox_.visibilityDependsOn(approximate_, [](auto l) { return l; });

    trackingInport_.setOptional(true);

    approximate_.onChange([this]() {
        if (approximate_ && !useComputeShaders_) {
            // Using CPU version of approximative algorithm.
            LogWarn("Compute shaders are not supported. Approximative depth of field "
                    << "post-processing may be slow.");
        }
    });

    addSampleShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidOutput); });
    averageLightFieldShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidOutput); });

    if (useComputeShaders_) {
        addToLightFieldShader_.build();
        addToLightFieldShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidOutput); });
    }
}

void DepthOfField::clickToFocus(Event* e) {
    auto mouseEvent = static_cast<MouseEvent*>(e);
    size2_t clickPos(mouseEvent->x(), mouseEvent->y());
    double depthNdc =
        inport_.getData()->getDepthLayer()->getRepresentation<LayerRAM>()->getAsNormalizedDouble(
            clickPos);
    focusDepth_.set(ndcToWorldDepth(depthNdc));
}

void DepthOfField::process() {
    int maxEvalCount =
        static_cast<int>(approximate_ ? viewCountApprox_.get() : viewCountExact_.get());
    std::shared_ptr<const Image> img = inport_.getData();
    size2_t dim = img->getDimensions();
    TextureUnitContainer cont;

    if (evalCount_ >= maxEvalCount) {
        // Catch final network evaluation caused by camera resetting
        evalCount_ = -1;
        return;
    } else if (evalCount_ == -1) {
        setupRecursion(dim, maxEvalCount, img);
        evalCount_ = 0;
        return;
    }

    SkewedPerspectiveCamera* camera = dynamic_cast<SkewedPerspectiveCamera*>(&camera_.get());
    double focusDepth = calculateFocusDepth();

    if (approximate_) {
        vec2 cameraPos = calculatePeripheralCameraPos(evalCount_, maxEvalCount);
        double fovy = glm::radians(camera->getFovy());
        if (useComputeShaders_) {
            warpToLightfieldGPU(cont, fovy, focusDepth, dim, cameraPos);
        } else {
            warpToLightfieldCPU(img, fovy, focusDepth, dim, cameraPos);
        }
    } else {
        addToAccumulationBuffer(img, cont);
    }

    if (evalCount_ < maxEvalCount - 1) {
        // Prepare camera for rendering again
        if (!approximate_) {
            std::swap(nextOutImg_, prevOutImg_);
        }
        evalCount_++;
        dispatchFront([this, camera, maxEvalCount, focusDepth]() {
            moveCamera(camera, maxEvalCount, focusDepth);
        });
    } else {
        // Set output
        if (approximate_) {
            synthesizeLightfield(cont);
        } else {
            outport_.setData(nextOutImg_);
        }

        // Reset camera
        evalCount_ = maxEvalCount;
        dispatchFront([this]() { camera_.setCamera(std::unique_ptr<Camera>(ogCamera_->clone())); });
    }
}

void DepthOfField::setupRecursion(size2_t dim, size_t maxEvalCount,
                                  std::shared_ptr<const Image> img) {
    ogCamera_.reset(camera_.get().clone());
    if (camera_.get().getClassIdentifier() != PerspectiveCamera::classIdentifier) {
        LogWarn("Intended for use with Perspective Camera. Unexpected behavior may follow.");
    }

    if (approximate_) {
        // Prepare light field
        size3_t dimLightField(dim.x, dim.y, viewCountApprox_.get() + simViewCountApprox_.get());
        if (!lightField_ || lightField_->getDimensions() != dimLightField) {
            lightField_ = std::make_shared<Volume>(dimLightField, DataFormat<vec4>::get());
            lightFieldDepth_ = std::make_shared<Volume>(dimLightField, DataFormat<float>::get());
        }
        float* lightFieldDepthData = static_cast<float*>(
            lightFieldDepth_->getEditableRepresentation<VolumeRAM>()->getData());
        std::fill(lightFieldDepthData,
                  lightFieldDepthData + dimLightField.x * dimLightField.y * dimLightField.z, -1.f);

        // Prepare Halton sequence
        if (haltonX_.size() != simViewCountApprox_.get()) {
            haltonX_ = util::haltonSequence<float>(2, simViewCountApprox_.get());
        }
        if (!haltonImg_ || haltonImg_->getDimensions().x != simViewCountApprox_.get()) {
            haltonImg_ = std::make_shared<Image>(size2_t(simViewCountApprox_.get(), 1),
                                                 DataFormat<float>::get());
            float* haltonData = static_cast<float*>(
                haltonImg_->getColorLayer()->getEditableRepresentation<LayerRAM>()->getData());
            std::copy(haltonX_.begin(), haltonX_.end(), haltonData);
        }

    } else {
        // Reset accumulation buffer
        prevOutImg_ = std::make_shared<Image>(dim, DataFormat<vec4>::get());
        nextOutImg_ = std::make_shared<Image>(dim, DataFormat<vec4>::get());

        // Prepare Halton sequences
        if (haltonX_.size() != maxEvalCount) {
            haltonX_ = util::haltonSequence<float>(2, maxEvalCount);
        }
        if (haltonY_.size() != maxEvalCount) {
            haltonY_ = util::haltonSequence<float>(3, maxEvalCount);
        }
    }

    // Update focus depth and aperture ranges based on the depths in the scene, ignoring
    // background points and including the current value.
    const float* inputDepthData = static_cast<const float*>(
        img->getRepresentation<ImageRAM>()->getDepthLayerRAM()->getData());
    const auto minmax = std::accumulate(
        inputDepthData, inputDepthData + dim.x * dim.y, std::make_pair(1.0f, 0.0f),
        [](const std::pair<float, float>& acc, const float v) -> std::pair<float, float> {
            return {std::min(acc.first, v), (v < 1) ? std::max(acc.second, v) : acc.second};
        });
    const auto minDepthWorld = ndcToWorldDepth(minmax.first);
    const auto maxDepthWorld = ndcToWorldDepth(minmax.second);

    dispatchFront([this, minDepthWorld, maxDepthWorld]() {
        NetworkLock lock(this);

        focusDepth_.set(focusDepth_.get(), std::min(minDepthWorld, focusDepth_.get()),
                        std::max(maxDepthWorld, focusDepth_.get()),
                        (maxDepthWorld - minDepthWorld) / 100.0);

        int depthScale = static_cast<int>(floor(log10(minDepthWorld)));
        double minAperture = pow(10, depthScale - 1);
        double maxAperture = pow(10, depthScale);
        aperture_.set(aperture_.get(), std::min(minAperture, aperture_.get()),
                      std::max(maxAperture, aperture_.get()), (maxAperture - minAperture) / 90.0);

        camera_.setCamera(SkewedPerspectiveCamera::classIdentifier);
    });
}

double DepthOfField::calculateFocusDepth() {
    if (manualFocus_ || !trackingInport_.hasData()) {
        return focusDepth_.get();
    } else {
        vec3 lookFrom = ogCamera_->getLookFrom();
        vec3 lookTo = ogCamera_->getLookTo();
        vec3 front = glm::normalize(lookTo - lookFrom);

        auto mesh = trackingInport_.getData();
        const vec3 focusPoint = mesh->getBuffer(0)->getRepresentation<BufferRAM>()->getAsDVec3(0);
        return glm::length(glm::dot(focusPoint - lookFrom, front) * front);
    }
}

vec2 DepthOfField::calculatePeripheralCameraPos(int evalCount, int maxEvalCount) {
    if (evalCount == 0) {
        return vec2(0.0f, 0.0f);
    } else {
        double currAngle = 2.0 * std::numbers::pi * (static_cast<double>(evalCount) - 0.5) /
                           static_cast<double>(maxEvalCount - 1);
        return static_cast<vec2>(aperture_.get() / 2.0 *
                                 dvec2(glm::cos(currAngle), glm::sin(currAngle)));
    }
}

void DepthOfField::addToAccumulationBuffer(std::shared_ptr<const Image> img,
                                           TextureUnitContainer& cont) {
    utilgl::activateTargetAndCopySource(*nextOutImg_, inport_, ImageType::ColorOnly);
    addSampleShader_.activate();

    img->getRepresentation<ImageGL>();
    prevOutImg_->getRepresentation<ImageGL>();
    utilgl::bindAndSetUniforms(addSampleShader_, cont, *img, "newImg", ImageType::ColorOnly);
    utilgl::bindAndSetUniforms(addSampleShader_, cont, *prevOutImg_, "oldImg",
                               ImageType::ColorOnly);
    addSampleShader_.setUniform("nOldImages", evalCount_);

    utilgl::singleDrawImagePlaneRect();
    addSampleShader_.deactivate();
}

void DepthOfField::warpToLightfieldGPU(TextureUnitContainer& cont, double fovy, double focusDepth,
                                       size2_t dim, vec2 cameraPos) {
    glActiveTexture(GL_TEXTURE);
    auto lightFieldGL = lightField_->getEditableRepresentation<VolumeGL>();
    auto lightFieldTexHandle = lightFieldGL->getTexture()->getID();
    glBindImageTexture(0, lightFieldTexHandle, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    auto lightFieldDepthGL = lightFieldDepth_->getEditableRepresentation<VolumeGL>();
    auto lightFieldDepthTexHandle = lightFieldDepthGL->getTexture()->getID();
    glBindImageTexture(1, lightFieldDepthTexHandle, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

    addToLightFieldShader_.activate();
    utilgl::bindAndSetUniforms(addToLightFieldShader_, cont, inport_, ImageType::ColorDepth);
    utilgl::bindAndSetUniforms(addToLightFieldShader_, cont, *haltonImg_, "halton",
                               ImageType::ColorOnly);

    addToLightFieldShader_.setUniform("nearClip", static_cast<float>(camera_.getNearPlaneDist()));
    addToLightFieldShader_.setUniform("farClip", static_cast<float>(camera_.getFarPlaneDist()));
    addToLightFieldShader_.setUniform("evalCount", static_cast<int>(evalCount_));
    addToLightFieldShader_.setUniform("aperture", static_cast<float>(aperture_.get()));
    addToLightFieldShader_.setUniform("nViews", static_cast<int>(viewCountApprox_.get()));
    addToLightFieldShader_.setUniform("nSimViews", static_cast<int>(simViewCountApprox_.get()));
    addToLightFieldShader_.setUniform("fovy", static_cast<float>(fovy));
    addToLightFieldShader_.setUniform("focusDepth", static_cast<float>(focusDepth));
    addToLightFieldShader_.setUniform("dim", static_cast<vec2>(dim));
    addToLightFieldShader_.setUniform("cameraPos", cameraPos);

    lightFieldGL->getTexture()->bind();
    addToLightFieldShader_.setUniform("lightField", 0);
    lightFieldDepthGL->getTexture()->bind();
    addToLightFieldShader_.setUniform("lightFieldDepth", 1);

    if (evalCount_ == 0) {
        // Warp central (first) view to all simulated views to approximate the view from each point.
        addToLightFieldShader_.setUniform("segmentStart", 0);
        glDispatchCompute(static_cast<GLuint>(dim.x), static_cast<GLuint>(dim.y),
                          static_cast<GLuint>(simViewCountApprox_.get()));
    } else {
        // Warp peripheral views to a circle segment of simulated views to fill visibility holes
        // left after warping the central view.
        const double segmentWidth =
            double(simViewCountApprox_.get()) / double(viewCountApprox_.get() - 1);
        const int start = static_cast<int>((evalCount_ - 1) * segmentWidth);
        const int stop = static_cast<int>(evalCount_ * segmentWidth);
        addToLightFieldShader_.setUniform("segmentStart", start);
        glDispatchCompute(static_cast<GLuint>(dim.x), static_cast<GLuint>(dim.y),
                          static_cast<GLuint>(stop - start));
    }

    addToLightFieldShader_.deactivate();
}

void DepthOfField::warpToLightfieldCPU(std::shared_ptr<const Image> img, double fovy,
                                       double focusDepth, size2_t dim, vec2 cameraPos) {
    const LayerRAM* inColor = img->getRepresentation<ImageRAM>()->getColorLayerRAM();
    const LayerRAM* inDepth = img->getRepresentation<ImageRAM>()->getDepthLayerRAM();
    VolumeRAM* lightField = lightField_->getEditableRepresentation<VolumeRAM>();
    VolumeRAM* lightFieldDepth = lightFieldDepth_->getEditableRepresentation<VolumeRAM>();

    for (size_t x = 0; x < dim.x; x++) {
        for (size_t y = 0; y < dim.y; y++) {
            vec2 screenPos(x, y);
            double zNdc = inDepth->getAsDouble(screenPos);
            double zWorld = ndcToWorldDepth(zNdc);
            vec4 color = inColor->getAsNormalizedDVec4(screenPos);

            size3_t pos(x, y, evalCount_);
            lightField->setFromDVec4(pos, color);
            lightFieldDepth->setFromDouble(pos, zWorld);

            if (evalCount_ == 0) {
                // Warp central (first) view to all simulated views to approximate the view from
                // each point.
                for (size_t i = 0; i < simViewCountApprox_.get(); i++) {
                    warp(cameraPos, screenPos, color, zWorld, i, lightField, lightFieldDepth, fovy,
                         focusDepth);
                }
            } else {
                // Warp peripheral views to a circle segment of simulated views to fill visibility
                // holes left after warping the central view.
                const double segmentWidth =
                    double(simViewCountApprox_.get()) / double(viewCountApprox_.get() - 1);
                const int start = static_cast<int>((evalCount_ - 1) * segmentWidth);
                const int stop = static_cast<int>(evalCount_ * segmentWidth);
                for (int i = start; i < stop; i++) {
                    warp(cameraPos, screenPos, color, zWorld, i, lightField, lightFieldDepth, fovy,
                         focusDepth);
                }
            }
        }
    }
}

void DepthOfField::warp(vec2 cameraPos, vec2 screenPos, vec4 color, double zWorld, size_t viewIndex,
                        VolumeRAM* lightField, VolumeRAM* lightFieldDepth, double fovy,
                        double focusDepth) {
    double radius = aperture_.get() / 2.0 * sqrt(haltonX_[viewIndex]);
    double angle = double(viewIndex) / double(simViewCountApprox_.get()) * 2.0 * std::numbers::pi;
    vec2 simCameraPos = radius * vec2(glm::cos(angle), glm::sin(angle));

    vec2 disparity = (1.0 / zWorld - 1.0 / focusDepth) * (cameraPos - simCameraPos);
    size3_t dimLightField = lightField_->getDimensions();
    vec2 simScreenpos = screenPos + disparity * dimLightField.y / (2.0 * std::tan(fovy / 2.0));
    ivec3 pos(round(simScreenpos.x), round(simScreenpos.y), viewCountApprox_.get() + viewIndex);
    if (pos.x < 0 || pos.x >= static_cast<int>(dimLightField.x) || pos.y < 0 ||
        pos.y >= static_cast<int>(dimLightField.y) || pos.z < 0 ||
        pos.z >= static_cast<int>(dimLightField.z))
        return;

    double currDepth = lightFieldDepth->getAsDouble(pos);
    if (currDepth < 0 || currDepth > zWorld + 0.01) {
        lightField->setFromDVec4(pos, color);
        lightFieldDepth->setFromDouble(pos, zWorld);
    }
}

void DepthOfField::synthesizeLightfield(TextureUnitContainer& cont) {
    utilgl::activateTargetAndCopySource(outport_, inport_, ImageType::ColorOnly);
    averageLightFieldShader_.activate();

    lightField_->getRepresentation<VolumeGL>();
    lightFieldDepth_->getRepresentation<VolumeGL>();

    utilgl::bindAndSetUniforms(averageLightFieldShader_, cont, *lightField_, "lightField");
    utilgl::bindAndSetUniforms(averageLightFieldShader_, cont, *lightFieldDepth_,
                               "lightFieldDepth");
    averageLightFieldShader_.setUniform("dim", vec3(lightField_->getDimensions()));

    utilgl::singleDrawImagePlaneRect();

    averageLightFieldShader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

void DepthOfField::moveCamera(SkewedPerspectiveCamera* camera, int maxEvalCount,
                              double focusDepth) {
    vec3 lookFrom = ogCamera_->getLookFrom();
    vec3 lookTo = ogCamera_->getLookTo();
    vec3 lookUp = ogCamera_->getLookUp();

    vec3 front = glm::normalize(lookTo - lookFrom);
    vec3 up = glm::normalize(lookUp - glm::dot(lookUp, front) * front);

    camera_.setLook(lookFrom, lookFrom + front * focusDepth, up);

    vec2 offset;
    if (approximate_) {
        offset = calculatePeripheralCameraPos(evalCount_ + 1, maxEvalCount);
    } else {
        double radius = aperture_.get() / 2.0 * sqrt(haltonX_[evalCount_]);
        double angle = 2.0 * std::numbers::pi * haltonY_[evalCount_];
        offset = static_cast<vec2>(radius * dvec2(glm::cos(angle), glm::sin(angle)));
    }
    camera->setOffset(offset);
}

double DepthOfField::ndcToWorldDepth(double depthNdc) {
    double nearClip = camera_.getNearPlaneDist();
    double farClip = camera_.getFarPlaneDist();
    return farClip * nearClip / (farClip - depthNdc * (farClip - nearClip));
}
}  // namespace inviwo
