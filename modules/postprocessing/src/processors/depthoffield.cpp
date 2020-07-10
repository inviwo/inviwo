/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <inviwo/core/interaction/events/mouseevent.h>
#include <modules/opengl/openglcapabilities.h>

namespace inviwo {

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
    , aperture_("aperture", "Aperture", 0.5f, 0.01f, 1.f, 0.01f)
    , focusDepth_("focusDepth", "Focus depth", 8.f, 1.0f, 15.0f, 0.1f)
    , manualFocus_("manualFocus", "Manual focus", true)
    , approximate_("approximate", "Approximate", false)
    , viewCountExact_("viewCountExact", "View count", 40, 10, 200)
    , viewCountApprox_("viewCountApprox", "Rendered view count", 5, 1, 12)
    , simViewCountApprox_("simViewCountApprox", "Simulated view count", 40, 10, 200)
    , clickToFocus_(
          "clickToFocus", "Click to focus", [this](Event* e) { clickToFocus(e); },
          MouseButton::Left, MouseState::Press, KeyModifier::Control)
    , camera_("camera", "Camera")
    , evalCount_(0)
    , useComputeShaders_(OpenGLCapabilities::getOpenGLVersion() >= 430)
    , ogCamera_{}
    , addSampleShader_("fullscreenquad.vert", "dof_exact.frag")
    , addToLightFieldShader_({{ShaderType::Compute, "dof_approx.comp"}})
    , averageLightfieldShader_("fullscreenquad.vert", "dof_approx.frag") {

    addPort(inport_);
    addPort(trackingInport_);
    addPort(outport_);
    addProperties(aperture_, focusDepth_, manualFocus_, approximate_, viewCountExact_,
                  viewCountApprox_, simViewCountApprox_, clickToFocus_, camera_);
    setApproximate(approximate_);

    trackingInport_.setOptional(true);

    approximate_.onChange([this]() { setApproximate(approximate_); });

    manualFocus_.onChange([this]() {
        focusDepth_.setReadOnly(!manualFocus_);
        clickToFocus_.setReadOnly(!manualFocus_);
    });

    addSampleShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidOutput); });
    addToLightFieldShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidOutput); });
    averageLightfieldShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidOutput); });
}

void DepthOfField::setApproximate(bool approximate) {
    viewCountExact_.setVisible(!approximate);
    viewCountApprox_.setVisible(approximate);
    simViewCountApprox_.setVisible(approximate);
    if (approximate && !useComputeShaders_) {
        // Using CPU version of approximative algorithm.
        LogWarn("Compute shaders are not supported. Approximative depth of field post-processing "
                << "may be slow.");
    }
}

void DepthOfField::clickToFocus(Event* e) {
    if (!manualFocus_) return;

    auto mouseEvent = static_cast<MouseEvent*>(e);
    size2_t clickPos(mouseEvent->x(), mouseEvent->y());
    double depthNdc =
        inport_.getData()->getDepthLayer()->getRepresentation<LayerRAM>()->getAsNormalizedDouble(
            clickPos);

    double nearClip = camera_.getNearPlaneDist();
    double farClip = camera_.getFarPlaneDist();
    double depthEye = farClip * nearClip / (farClip - depthNdc * (farClip - nearClip));
    focusDepth_.set(depthEye);
}

void DepthOfField::process() {
    int maxEvalCount = approximate_ ? viewCountApprox_.get() : viewCountExact_.get();
    std::shared_ptr<const Image> img = inport_.getData();
    size2_t dim = img->getDimensions();
    TextureUnitContainer cont;

    if (evalCount_ >= maxEvalCount) {
        // Catch final network evaluation caused by camera resetting
        evalCount_ = 0;
        return;
    } else if (evalCount_ == 0) {
        setupRecursion(dim, maxEvalCount, img);
    }

    SkewedPerspectiveCamera* camera = dynamic_cast<SkewedPerspectiveCamera*>(&camera_.get());
    double focusDepth = calculateFocusDepth();

    if (approximate_) {
        vec2 cameraPos = calculateCurrentCameraPos(maxEvalCount);
        double nearClip = camera_.getNearPlaneDist();
        double farClip = camera_.getFarPlaneDist();
        double fovy = glm::radians(camera->getFovy());
        if (useComputeShaders_) {
            warpToLightfieldGPU(cont, nearClip, farClip, fovy, focusDepth, dim, cameraPos);
        } else {
            warpToLightfieldCPU(img, nearClip, farClip, fovy, focusDepth, dim, cameraPos);
        }
    } else {
        addToAccumulationBuffer(img, cont);
    }

    evalCount_++;
    if (evalCount_ < maxEvalCount) {
        // Prepare camera for rendering again
        if (!approximate_) {
            nextOutImg_->copyRepresentationsTo(prevOutImg_.get());
        }
        moveCamera(camera, maxEvalCount, focusDepth);
    } else {
        // Set output
        if (approximate_) {
            synthesizeLightfield(cont);
        } else {
            outport_.setData(nextOutImg_);
        }

        // Reset camera
        if (ogType_ == SkewedPerspectiveCamera::classIdentifier) evalCount_ = 0;
        camera_.setCamera(std::unique_ptr<Camera>(ogCamera_->clone()));
    }
}

void DepthOfField::setupRecursion(size2_t dim, int maxEvalCount, std::shared_ptr<const Image> img) {
    ogCamera_.reset(camera_.get().clone());
    ogType_ = camera_.get().getClassIdentifier();
    if (ogType_ != PerspectiveCamera::classIdentifier) {
        LogWarn("Intended for use with Perspective Camera. Unexpected behavior may follow.");
    }
    camera_.setCamera(SkewedPerspectiveCamera::classIdentifier);

    if (approximate_) {
        // Prepare light field
        dimLightField_ = size3_t(dim.x, dim.y, viewCountApprox_.get() + simViewCountApprox_.get());
        lightField_ = std::make_shared<Volume>(dimLightField_, DataFormat<vec4>::get());
        lightFieldDepth_ = std::make_shared<Volume>(dimLightField_, DataFormat<float>::get());
        float* lightFieldDepthData = static_cast<float*>(
            lightFieldDepth_->getEditableRepresentation<VolumeRAM>()->getData());
        std::fill(lightFieldDepthData,
                  lightFieldDepthData + dimLightField_.x * dimLightField_.y * dimLightField_.z,
                  -1.f);

        // Prepare Halton sequence
        haltonX_ = util::haltonSequence<float>(2, simViewCountApprox_.get());
        haltonImg_ = std::make_shared<Image>(size2_t(simViewCountApprox_.get(), 1),
                                             DataFormat<float>::get());
        float* haltonData = static_cast<float*>(
            haltonImg_->getColorLayer()->getEditableRepresentation<LayerRAM>()->getData());
        std::copy(haltonX_.begin(), haltonX_.end(), haltonData);
    } else {
        // Prepare accumulation buffer
        prevOutImg_ = std::make_shared<Image>(dim, DataFormat<vec4>::get());
        nextOutImg_ = std::make_shared<Image>(dim, DataFormat<vec4>::get());

        // Prepare Halton sequences
        haltonX_ = util::haltonSequence<float>(2, maxEvalCount);
        haltonY_ = util::haltonSequence<float>(3, maxEvalCount);
    }

    // Update focus depth and aperture ranges based on the depths in the scene, ignoring
    // background points and including the current value.
    const float* inputDepthData = static_cast<const float*>(
        img->getRepresentation<ImageRAM>()->getDepthLayerRAM()->getData());
    float minDepthNdc = 1;
    float maxDepthNdc = 0;
    for (const float* f = inputDepthData; f < inputDepthData + dim.x * dim.y; f++) {
        minDepthNdc = std::min(minDepthNdc, *f);
        if (*f < 1) {
            maxDepthNdc = std::max(maxDepthNdc, *f);
        }
    }

    double nearClip = camera_.getNearPlaneDist();
    double farClip = camera_.getFarPlaneDist();
    float minDepthWorld = nearClip * farClip / (minDepthNdc * (nearClip - farClip) + farClip);
    float maxDepthWorld = nearClip * farClip / (maxDepthNdc * (nearClip - farClip) + farClip);

    focusDepth_.setMinValue(std::min(minDepthWorld, focusDepth_.get()));
    focusDepth_.setMaxValue(std::max(maxDepthWorld, focusDepth_.get()));
    focusDepth_.set(focusDepth_.get(), std::min(minDepthWorld, focusDepth_.get()),
                    std::max(maxDepthWorld, focusDepth_.get()),
                    (maxDepthWorld - minDepthWorld) / 100.0);

    int depthScale = floor(log10(minDepthWorld));
    float minAperture = pow(10, depthScale - 1);
    float maxAperture = pow(10, depthScale);
    aperture_.set(aperture_.get(), std::min(minAperture, aperture_.get()),
                  std::max(maxAperture, aperture_.get()), (maxAperture - minAperture) / 90.0);
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

vec2 DepthOfField::calculateCurrentCameraPos(int maxEvalCount) {
    if (evalCount_ == 0) {
        return vec2(0, 0);
    } else {
        float currAngle = 2.0 * M_PI * (float(evalCount_) - 0.5) / float(maxEvalCount - 1);
        return aperture_.get() / 2.0 * vec2(glm::cos(currAngle), glm::sin(currAngle));
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

void DepthOfField::warpToLightfieldGPU(TextureUnitContainer& cont, double nearClip, double farClip,
                                       double fovy, double focusDepth, size2_t dim,
                                       vec2 cameraPos) {
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

    addToLightFieldShader_.setUniform("nearClip", float(nearClip));
    addToLightFieldShader_.setUniform("farClip", float(farClip));
    addToLightFieldShader_.setUniform("evalCount", int(evalCount_));
    addToLightFieldShader_.setUniform("aperture", float(aperture_.get()));
    addToLightFieldShader_.setUniform("nViews", int(viewCountApprox_.get()));
    addToLightFieldShader_.setUniform("nSimViews", int(simViewCountApprox_.get()));
    addToLightFieldShader_.setUniform("fovy", float(fovy));
    addToLightFieldShader_.setUniform("focusDepth", float(focusDepth));
    addToLightFieldShader_.setUniform("dim", vec2(dim));
    addToLightFieldShader_.setUniform("cameraPos", cameraPos);

    lightFieldGL->getTexture()->bind();
    addToLightFieldShader_.setUniform("lightField", 0);
    lightFieldDepthGL->getTexture()->bind();
    addToLightFieldShader_.setUniform("lightFieldDepth", 1);

    if (evalCount_ == 0) {
        // Warp central (first) view to all simulated views.
        addToLightFieldShader_.setUniform("segmentStart", 0);
        glDispatchCompute(dim.x, dim.y, simViewCountApprox_.get());
    } else {
        // Warp peripheral views to a circle segment of simulated views.
        double segmentWidth =
            double(simViewCountApprox_.get()) / double(viewCountApprox_.get() - 1);
        addToLightFieldShader_.setUniform("segmentStart", int((evalCount_ - 1) * segmentWidth));
        glDispatchCompute(dim.x, dim.y, int(segmentWidth));
    }

    addToLightFieldShader_.deactivate();
}

void DepthOfField::warpToLightfieldCPU(std::shared_ptr<const Image> img, double nearClip,
                                       double farClip, double fovy, double focusDepth, size2_t dim,
                                       vec2 cameraPos) {
    const LayerRAM* inColor = img->getRepresentation<ImageRAM>()->getColorLayerRAM();
    const LayerRAM* inDepth = img->getRepresentation<ImageRAM>()->getDepthLayerRAM();
    VolumeRAM* lightField = lightField_->getEditableRepresentation<VolumeRAM>();
    VolumeRAM* lightFieldDepth = lightFieldDepth_->getEditableRepresentation<VolumeRAM>();

    for (size_t x = 0; x < dim.x; x++) {
        for (size_t y = 0; y < dim.y; y++) {
            vec2 screenPos(x, y);
            double zNdc = inDepth->getAsDouble(screenPos);
            double zWorld = nearClip * farClip / (zNdc * (nearClip - farClip) + farClip);
            vec4 color = inColor->getAsNormalizedDVec4(screenPos);

            size3_t pos(x, y, evalCount_);
            lightField->setFromDVec4(pos, color);
            lightFieldDepth->setFromDouble(pos, zWorld);

            if (evalCount_ == 0) {
                // Warp central (first) view to all simulated views.
                for (size_t i = 0; i < simViewCountApprox_.get(); i++) {
                    warp(cameraPos, screenPos, color, zWorld, i, lightField, lightFieldDepth, fovy,
                         focusDepth);
                }
            } else {
                // Warp peripheral views to a circle segment of simulated views.
                double segmentWidth =
                    double(simViewCountApprox_.get()) / double(viewCountApprox_.get() - 1);
                int start = (evalCount_ - 1) * segmentWidth;
                int stop = evalCount_ * segmentWidth;
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
    double angle = double(viewIndex) / double(simViewCountApprox_.get()) * 2.0 * M_PI;
    vec2 simCameraPos = radius * vec2(glm::cos(angle), glm::sin(angle));

    vec2 disparity = (1.0 / zWorld - 1.0 / focusDepth) * (cameraPos - simCameraPos);
    vec2 simScreenpos = screenPos + disparity * dimLightField_.y / (2.0 * std::tan(fovy / 2.0));
    size3_t pos(round(simScreenpos.x), round(simScreenpos.y), viewCountApprox_.get() + viewIndex);
    if (pos.x < 0 || pos.x >= dimLightField_.x || pos.y < 0 || pos.y >= dimLightField_.y ||
        pos.z < 0 || pos.z >= dimLightField_.z)
        return;

    double currDepth = lightFieldDepth->getAsDouble(pos);
    if (currDepth < 0 || currDepth > zWorld + 0.01) {
        lightField->setFromDVec4(pos, color);
        lightFieldDepth->setFromDouble(pos, zWorld);
    }
}

void DepthOfField::synthesizeLightfield(TextureUnitContainer& cont) {
    utilgl::activateTargetAndCopySource(outport_, inport_, ImageType::ColorOnly);
    averageLightfieldShader_.activate();

    lightField_->getRepresentation<VolumeGL>();
    lightFieldDepth_->getRepresentation<VolumeGL>();

    utilgl::bindAndSetUniforms(averageLightfieldShader_, cont, *lightField_, "lightField");
    utilgl::bindAndSetUniforms(averageLightfieldShader_, cont, *lightFieldDepth_,
                               "lightFieldDepth");
    averageLightfieldShader_.setUniform("dim", vec3(dimLightField_));

    utilgl::singleDrawImagePlaneRect();

    averageLightfieldShader_.deactivate();
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

    float radius = approximate_ ? aperture_.get() / 2.0
                                : aperture_.get() / 2.0 * sqrt(haltonX_[evalCount_ - 1]);
    float angle = 2.0 * M_PI *
                  (approximate_ ? (float(evalCount_) - 0.5) / float(maxEvalCount - 1)
                                : haltonY_[evalCount_ - 1]);
    vec2 offset = radius * vec2(glm::cos(angle), glm::sin(angle));
    camera->setOffset(offset);
}
}  // namespace inviwo
