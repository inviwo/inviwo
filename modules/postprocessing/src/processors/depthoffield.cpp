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
    , outport_("outport")
    , aperture_("aperture", "Aperture", 0.4f, 0.01f, 1.f, 0.01f)
    , focusDepth_("focusDepth", "Focus depth", 5.f, 1.0f, 15.0f, 0.1f)
    , approximate_("approximate", "Approximate", true)
    , viewCountExact_("viewCountExact", "View count", 20, 10, 200)
    , viewCountApprox_("viewCountApprox", "Rendered view count", 5, 1, 12)
    , simViewCountApprox_("simViewCountApprox", "Simulated view count", 10, 10, 200)
    , camera_("camera", "Camera")
    , evalCount_(0)
    , ogCamera_{}
    , addSampleShader_("fullscreenquad.vert", "dof_exact.frag")
    , addToLightFieldShader_({{ShaderType::Compute, "dof_approx.comp"}})
    , averageLightfieldShader_("fullscreenquad.vert", "dof_approx.frag") {

    addPort(inport_);
    addPort(outport_);
    addProperties(aperture_, focusDepth_, approximate_, viewCountApprox_, simViewCountApprox_, camera_);

    approximate_.onChange([this]() {
        if (approximate_) {
            // Use approximative algorithm
            removeProperty(viewCountExact_);
            insertProperty(3, viewCountApprox_);
            insertProperty(4, simViewCountApprox_);
        } else {
            // Use exact algorithm
            removeProperty(viewCountApprox_);
            removeProperty(simViewCountApprox_);
            insertProperty(3, viewCountExact_);
        }
    });

    addSampleShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidOutput); });
    addToLightFieldShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidOutput); });
    averageLightfieldShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidOutput); });
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
        // Set up for a new session
        ogCamera_.reset(camera_.get().clone());
        camera_.setCamera(SkewedPerspectiveCamera::classIdentifier);
        
        if (approximate_) {
            // Prepare light field
            dimLightField_ = size3_t(dim.x, dim.y, viewCountApprox_.get() + simViewCountApprox_.get());
            lightField_ = std::make_shared<Volume>(dimLightField_, DataFormat<vec4>::get());
            lightFieldDepth_ = std::make_shared<Volume>(dimLightField_, DataFormat<float>::get());
            float* lightFieldDepthData = static_cast<float*>(
                lightFieldDepth_->getEditableRepresentation<VolumeRAM>()->getData());
            std::fill(lightFieldDepthData, 
                lightFieldDepthData + dimLightField_.x * dimLightField_.y * dimLightField_.z, -1.f);

            // Prepare Halton sequence
            haltonX_ = util::haltonSequence<float>(2, simViewCountApprox_.get());
            haltonImg_ = std::make_shared<Image>(
                size2_t(simViewCountApprox_.get(), 1), DataFormat<float>::get());
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
    }

    SkewedPerspectiveCamera* camera = dynamic_cast<SkewedPerspectiveCamera*>(&camera_.get());

    if (approximate_) {
        // Find position of current camera
        vec2 cameraPos;
        if (evalCount_ == 0) {
            cameraPos = vec2(0, 0);
        } else {
            float currAngle = 2.0 * M_PI * (float(evalCount_) - 0.5) / float(maxEvalCount);
            cameraPos = aperture_.get() / 2.0 * vec2(glm::cos(currAngle), glm::sin(currAngle));
        }

        double nearClip = camera_.getNearPlaneDist();
        double farClip = camera_.getFarPlaneDist();
        double fovy = glm::radians(camera->getFovy());

        bool useComputeShaders = false;     // TODO: Change depending on support.

        // Warp rendered image to light field
        if (useComputeShaders) {
            glActiveTexture(GL_TEXTURE);
            auto lightFieldGL = lightField_->getEditableRepresentation<VolumeGL>();
            auto lightFieldTexHandle = lightFieldGL->getTexture()->getID();
            glBindImageTexture(0, lightFieldTexHandle, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

            auto lightFieldDepthGL = lightFieldDepth_->getEditableRepresentation<VolumeGL>();
            auto lightFieldDepthTexHandle = lightFieldDepthGL->getTexture()->getID();
            glBindImageTexture(1, lightFieldDepthTexHandle, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

            addToLightFieldShader_.activate();
            utilgl::bindAndSetUniforms(addToLightFieldShader_, cont, inport_, ImageType::ColorDepth);
            utilgl::bindAndSetUniforms(addToLightFieldShader_, cont, *haltonImg_, "halton", ImageType::ColorOnly);

            addToLightFieldShader_.setUniform("nearClip", float(nearClip));
            addToLightFieldShader_.setUniform("farClip", float(farClip));
            addToLightFieldShader_.setUniform("evalCount", int(evalCount_));
            addToLightFieldShader_.setUniform("aperture", float(aperture_.get()));
            addToLightFieldShader_.setUniform("nViews", int(viewCountApprox_.get()));
            addToLightFieldShader_.setUniform("nSimViews", int(simViewCountApprox_.get()));
            addToLightFieldShader_.setUniform("fovy", float(fovy));
            addToLightFieldShader_.setUniform("focusDepth", float(focusDepth_.get()));
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
                double segmentWidth = double(simViewCountApprox_.get()) / double(viewCountApprox_.get() - 1);
                addToLightFieldShader_.setUniform("segmentStart", int((evalCount_ - 1) * segmentWidth));
                glDispatchCompute(dim.x, dim.y, int(segmentWidth));
            }

            addToLightFieldShader_.deactivate();
        } else {
            const LayerRAM* inColor = img->getRepresentation<ImageRAM>()->getColorLayerRAM();
            const LayerRAM* inDepth = img->getRepresentation<ImageRAM>()->getDepthLayerRAM();
            vec4* lightFieldData = static_cast<vec4*>(
                lightField_->getEditableRepresentation<VolumeRAM>()->getData());
            float* lightFieldDepthData = static_cast<float*>(
                lightFieldDepth_->getEditableRepresentation<VolumeRAM>()->getData());

            for (size_t x = 0; x < dim.x; x++) {
                for (size_t y = 0; y < dim.y; y++) {
                    vec2 screenPos(x, y);
                    double zNdc = inDepth->getAsDouble(screenPos);
                    double zWorld = nearClip * farClip / (zNdc * (nearClip - farClip) + farClip);
                    vec4 color = inColor->getAsNormalizedDVec4(screenPos);

                    size3_t pos(x, y, evalCount_);
                    lightFieldData[VolumeRAM::posToIndex(pos, dimLightField_)] = color;
                    lightFieldDepthData[VolumeRAM::posToIndex(pos, dimLightField_)] = zWorld;

                    if (evalCount_ == 0) {
                        // Warp central (first) view to all simulated views.
                        for (size_t i = 0; i < simViewCountApprox_.get(); i++ ) {
                            warp(cameraPos, screenPos, color, zWorld, i, lightFieldData, lightFieldDepthData, fovy);
                        }
                    } else {
                        // Warp peripheral views to a circle segment of simulated views.
                        double segmentWidth = double(simViewCountApprox_.get()) / double(viewCountApprox_.get() - 1);
                        int start = (evalCount_ - 1) * segmentWidth;
                        int stop = evalCount_ * segmentWidth;
                        for (int i = start; i < stop; i++) {
                            warp(cameraPos, screenPos, color, zWorld, i, lightFieldData, lightFieldDepthData, fovy);
                        }
                    }
                }
            }
        }
    } else {
        // Add new image to accumulation buffer
        utilgl::activateTargetAndCopySource(*nextOutImg_, inport_, ImageType::ColorOnly);
        addSampleShader_.activate();

        img->getRepresentation<ImageGL>();
        prevOutImg_->getRepresentation<ImageGL>();
        utilgl::bindAndSetUniforms(addSampleShader_, cont, *img, "newImg", ImageType::ColorOnly);
        utilgl::bindAndSetUniforms(addSampleShader_, cont, *prevOutImg_, "oldImg", ImageType::ColorOnly);
        addSampleShader_.setUniform("nOldImages", evalCount_);

        utilgl::singleDrawImagePlaneRect();
        addSampleShader_.deactivate();
    }

    evalCount_++;
    if (evalCount_ < maxEvalCount) {
        // Prepare camera for rendering again
        if (!approximate_) {
            nextOutImg_->copyRepresentationsTo(prevOutImg_.get());
        }

        vec3 lookFrom = ogCamera_->getLookFrom();
        vec3 lookTo = ogCamera_->getLookTo();
        vec3 lookUp = ogCamera_->getLookUp();

        vec3 front = glm::normalize(lookTo - lookFrom);
        vec3 up = glm::normalize(lookUp - glm::dot(lookUp, front) * front);

        camera_.setLook(lookFrom, lookFrom + front * focusDepth_.get(), up);

        float radius = approximate_ 
            ? aperture_.get() / 2.0
            : aperture_.get() / 2.0 * sqrt(haltonX_[evalCount_ - 1]);
        float angle = 2.0 * M_PI * (approximate_ 
            ?  (float(evalCount_) - 0.5) / float(maxEvalCount) 
            :  haltonY_[evalCount_ - 1]);
        vec2 offset = radius * vec2(glm::cos(angle), glm::sin(angle));
        camera->setOffset(offset);
    } else {
        // Set output
        if (approximate_) {
            utilgl::activateTargetAndCopySource(outport_, inport_, ImageType::ColorOnly);
            averageLightfieldShader_.activate();

            lightField_->getRepresentation<VolumeGL>();
            lightFieldDepth_->getRepresentation<VolumeGL>();

            utilgl::bindAndSetUniforms(averageLightfieldShader_, cont, *lightField_, "lightField");
            utilgl::bindAndSetUniforms(averageLightfieldShader_, cont, *lightFieldDepth_, "lightFieldDepth");
            averageLightfieldShader_.setUniform("dim", vec3(dimLightField_));

            utilgl::singleDrawImagePlaneRect();

            averageLightfieldShader_.deactivate();
            utilgl::deactivateCurrentTarget();
        } else {
            outport_.setData(nextOutImg_);
        }

        // Reset camera
        if (typeid(camera) == typeid(*ogCamera_)) evalCount_ = 0;
        camera_.setCamera(std::unique_ptr<Camera>(ogCamera_->clone()));
    }
}

void DepthOfField::warp(vec2 cameraPos, vec2 screenPos, vec4 color, double zWorld, size_t viewIndex,
                        vec4* lightFieldData, float* lightFieldDepthData, double fovy) {
    double radius = aperture_.get() / 2.0 * sqrt(haltonX_[viewIndex]);
    double angle = double(viewIndex) / double(simViewCountApprox_.get()) * 2.0 * M_PI;
    vec2 simCameraPos = radius * vec2(glm::cos(angle), glm::sin(angle));

    vec2 disparity = (1.0 / zWorld - 1.0 / focusDepth_.get()) * (cameraPos - simCameraPos);
    vec2 simScreenpos = screenPos + disparity * dimLightField_.y / (2.0 * std::tan(fovy / 2.0));
    if (simScreenpos.x < 0 || simScreenpos.x >= dimLightField_.x 
        || simScreenpos.y < 0 || simScreenpos.y >= dimLightField_.y) return;

    size3_t pos(round(simScreenpos.x), round(simScreenpos.y), viewCountApprox_.get() + viewIndex);
    float currDepth = lightFieldDepthData[VolumeRAM::posToIndex(pos, dimLightField_)];
    if (currDepth < 0 || currDepth > zWorld + 0.01) {
        lightFieldData[VolumeRAM::posToIndex(pos, dimLightField_)] = color;
        lightFieldDepthData[VolumeRAM::posToIndex(pos, dimLightField_)] = zWorld;
    }
}

}  // namespace inviwo
