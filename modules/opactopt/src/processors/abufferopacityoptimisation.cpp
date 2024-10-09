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

#include <modules/opactopt/processors/abufferopacityoptimisation.h>

#include <inviwo/core/algorithm/boundingbox.h>
#include <modules/oit/ports/rasterizationport.h>  // for RasterizationOutport
#include <modules/opengl/shader/shaderutils.h>    // for ImageInport, setUnif...

#include <iostream>
#include <fstream>
#include <rapidjson/document.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo AbufferOpacityOptimisation::processorInfo_{
    "org.inviwo.AbufferOpacityOptimisation",  // Class identifier
    "A-buffer Opacity Optimisation",          // Display name
    "Mesh Rendering",                         // Category
    CodeState::Experimental,                  // Code state
    Tags::GL,                                 // Tags
    "Performs approximate opacity optimisation using an A-buffer"
    " rendering approach.The processor takes a rasterisation as input,"
    " and optionally an importance volume and background"
    "texture. The output is an opacity optimised image."_help};

const ProcessorInfo AbufferOpacityOptimisation::getProcessorInfo() const { return processorInfo_; }

AbufferOpacityOptimisation::AbufferOpacityOptimisation()
    : q_{"q",
         "q",
         1.0f,
         0.0f,
         10000.0f,
         0.01f,
         InvalidationLevel::InvalidOutput,
         PropertySemantics::SpinBox}
    , r_{"r",
         "r",
         1.0f,
         0.0f,
         10000.0f,
         0.01f,
         InvalidationLevel::InvalidOutput,
         PropertySemantics::SpinBox}
    , lambda_{"lambda",
              "lambda",
              1.0f,
              0.0f,
              100.0f,
              0.01f,
              InvalidationLevel::InvalidOutput,
              PropertySemantics::SpinBox}
    , approximationProperties_{"approximationProperties", "Approximation Properties"}
    , approximationMethod_{"approximationMethod", "Approximation Method",
                           Approximations::generateApproximationStringOptions(), 0}
    , importanceVolume_{"importanceVolume", "Optional scalar field with importance data"_help}
    , importanceSumCoefficients_{"importanceSumCoefficients",
                                 "Importance sum coefficients",
                                 5,
                                 Approximations::approximations.at(approximationMethod_)
                                     .minCoefficients,
                                 Approximations::approximations.at(approximationMethod_)
                                     .maxCoefficients,
                                 Approximations::approximations.at(approximationMethod_).increment}
    , opticalDepthCoefficients_{"opticalDepthCoefficients",
                                "Optical depth coefficients",
                                5,
                                Approximations::approximations.at(approximationMethod_)
                                    .minCoefficients,
                                Approximations::approximations.at(approximationMethod_)
                                    .maxCoefficients,
                                Approximations::approximations.at(approximationMethod_).increment}
    , useExactBlending_{"useExactBlending", "Use exact blending", false}
    , normalisedBlending_{"normalisedBlending", "Normalised blending", true}
    , smoothing_{"smoothing", "Smoothing", false}
    , gaussianKernelRadius_{"gaussianKernelRadius", "Gaussian kernel radius", 3, 1, 50}
    , gaussianKernelSigma_{"gaussianKernelSigma", "Gaussian kernel sigma", 1, 0.001, 50}
    , debugCoords_{"debugCoords", "Coordinates", {0, 0}}
    , debugFileName_{"debugFile", "Debug file"}
    , debugApproximation_{"debugApproximation", "Debug and export"}
    , timingMode_{"timingMode", "Timing mode", 0, 0, 2, 1, InvalidationLevel::Valid}
    , timeTotal_{"timeTotal", "Total time", 0, 0, INT64_MAX, 1, InvalidationLevel::Valid}
    , timeSetup_{"timeSetup", "Setup time", 0, 0, INT64_MAX, 1, InvalidationLevel::Valid}
    , timeRasterisation_{"timeRasterisation",     "Rasterisation time", 0, 0, INT64_MAX, 1,
                         InvalidationLevel::Valid}
    , timeProjection_{"timeProjection",        "Time projection", 0, 0, INT64_MAX, 1,
                      InvalidationLevel::Valid}
    , timeSmoothing_{"timeSmoothing",         "Time smoothing", 0, 0, INT64_MAX, 1,
                     InvalidationLevel::Valid}
    , timeBlending_{"timeBlending", "Time blending", 0, 0, INT64_MAX, 1, InvalidationLevel::Valid}
    , nfrags_{"nfrags", "Fragments rendered", 0, 0, INT64_MAX, 1, InvalidationLevel::Valid} {

    flr_ = std::make_unique<AbufferOpacityOptimisationRenderer>(
        &Approximations::approximations.at(approximationMethod_), &camera_,
        importanceSumCoefficients_, opticalDepthCoefficients_);
    aoor_ = dynamic_cast<AbufferOpacityOptimisationRenderer*>(flr_.get());
    aoor_->setNormalisedBlending(normalisedBlending_);

    addPort(importanceVolume_).setOptional(true);
    importanceVolume_.onChange([this]() {
        if (importanceVolume_.isReady())
            aoor_->importanceVolume = &importanceVolume_;
        else {
            aoor_->importanceVolume = nullptr;
        }
        aoor_->importanceVolumeDirty = true;
    });

    illustrationSettings_.enabled_.setChecked(false);
    removeProperty(illustrationSettings_.enabled_);
    addProperties(q_, r_, lambda_, approximationProperties_);

    q_.onChange([this]() { aoor_->q = q_; });
    r_.onChange([this]() { aoor_->r = r_; });
    lambda_.onChange([this]() { aoor_->lambda = lambda_; });

    approximationMethod_.setDefault("fourier");
    approximationMethod_.set("fourier");

    approximationProperties_.addProperties(approximationMethod_, importanceSumCoefficients_,
                                           opticalDepthCoefficients_, useExactBlending_,
                                           normalisedBlending_, smoothing_, debugCoords_,
                                           debugFileName_, debugApproximation_);
    smoothing_.addProperties(gaussianKernelRadius_, gaussianKernelSigma_);

    std::vector<Int64Property*> timers{&timeSetup_, &timeRasterisation_, &timeProjection_,
                                       &timeSmoothing_, &timeBlending_};
    execTimer.setTimers(&timeTotal_, timers);
    aoor_->execTimer = &execTimer;
    aoor_->nfrags = &nfrags_;
    for (auto timer : timers) {
        addProperty(timer);
        timer->setVisible(false);
    }
    addProperties(timingMode_, timeTotal_, nfrags_);
    timingMode_.setVisible(false);
    timeTotal_.setVisible(false);
    nfrags_.setVisible(false);

    approximationMethod_.onChange([this]() {
        const auto* p = &Approximations::approximations.at(approximationMethod_);
        importanceSumCoefficients_.setMinValue(p->minCoefficients);
        importanceSumCoefficients_.setMaxValue(p->maxCoefficients);
        importanceSumCoefficients_.setIncrement(p->increment);
        opticalDepthCoefficients_.setMinValue(p->minCoefficients);
        opticalDepthCoefficients_.setMaxValue(p->maxCoefficients);
        opticalDepthCoefficients_.setIncrement(p->increment);
        aoor_->setDescriptor(p);
    });

    importanceSumCoefficients_.onChange(
        [this]() { aoor_->setImportanceSumCoeffs(importanceSumCoefficients_); });
    opticalDepthCoefficients_.onChange(
        [this]() { aoor_->setOpticalDepthCoeffs(opticalDepthCoefficients_); });
    useExactBlending_.onChange([this]() {
        aoor_->setExactBlending(useExactBlending_);
        opticalDepthCoefficients_.setVisible(!useExactBlending_);
        normalisedBlending_.setVisible(!useExactBlending_);
    });
    normalisedBlending_.onChange([this]() { aoor_->setNormalisedBlending(normalisedBlending_); });
    smoothing_.onChange([this]() { aoor_->smoothing = smoothing_; });
    gaussianKernelRadius_.onChange([this]() {
        aoor_->generateAndUploadGaussianKernel(gaussianKernelRadius_, gaussianKernelSigma_);
    });
    gaussianKernelSigma_.onChange([this]() {
        aoor_->generateAndUploadGaussianKernel(gaussianKernelRadius_, gaussianKernelSigma_);
    });

    debugApproximation_.onChange([this]() {
        if (!debugFileName_.get().empty()) {
            aoor_->debug = true;
            aoor_->debugCoords = debugCoords_;
        } else
            LogError("Debug file: Invalid file name");
    });

    debugCoords_.setMinValue({0, 0});
    debugCoords_.setMaxValue(ivec2(aoor_->screenSize_) - ivec2(1, 1));
    debugFileName_.setSelectedExtension(
        FileExtension::createFileExtensionFromString("Text file (*.txt)"));

    approximationProperties_.propertyModified();
}

void AbufferOpacityOptimisation::process() {
    execTimer.reset(timingMode_);
    execTimer.addCounter();

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
        aoor_->prePass(outport_.getDimensions());
        execTimer.addCounter();

        aoor_->beginCount();

        for (const auto& rasterization : rasterizations_) {
            if (rasterization->usesFragmentLists() == UseFragmentList::Yes) {
                rasterization->rasterize(outport_.getDimensions(), mat4(1.0));
            }
        }
        aoor_->endCount();

        utilgl::activateTargetAndCopySource(outport_, intermediateImage_);

        success = aoor_->postPass(false, &intermediateImage_);
    }
    utilgl::deactivateCurrentTarget();

    if (aoor_->debug) {
        aoor_->debug = false;
        aoor_->db_.exportDebugInfo(debugFileName_.get(), *(aoor_->ap_), q_, r_, lambda_);
    }

    debugCoords_.setMinValue({0, 0});
    debugCoords_.setMaxValue(ivec2(aoor_->screenSize_) - ivec2(1, 1));
}

}  // namespace inviwo
