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

#include <modules/opactopt/processors/opacityoptimiser.h>

#include <inviwo/core/algorithm/boundingbox.h>
#include <modules/oit/ports/rasterizationport.h>  // for RasterizationOutport

#include <modules/opactopt/rendering/approximateopacityoptimisationrenderer.h>
#include <modules/opactopt/rendering/decoupledopacityoptimisationrenderer.h>
#include <modules/opengl/shader/shaderutils.h>  // for ImageInport, setUnif...

#include <iostream>
#include <fstream>
#include <rapidjson/document.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo OpacityOptimiser::processorInfo_{
    "org.inviwo.OpacityOptimiser",  // Class identifier
    "Opacity Optimiser",            // Display name
    "Mesh Rendering",               // Category
    CodeState::Experimental,        // Code state
    Tags::GL,                       // Tags
    R"(The processor takes a rasterization as input, and together with a importance function
       input outputs an opacity optimised image.)"_unindentHelp};

const ProcessorInfo OpacityOptimiser::getProcessorInfo() const { return processorInfo_; }

OpacityOptimiser::OpacityOptimiser()
    : q_{"q", "q", 1.0f, 0.0f, 1000.0f, 0.01f}
    , r_{"r", "r", 1.0f, 0.0f, 1000.0f, 0.01f}
    , lambda_{"lambda", "lambda", 0.5f, 0.001f, 10.0f, 0.01f}
    , opacityOptimisationRenderer_{"opacityOptimisationRenderer",
                                   "Opacity Optimisation Renderer",
                                   {{"approximate", "Approximate", 0},
                                    {"decoupled", "Decoupled", 1}}}
    , approximationProperties_{"approximationProperties", "Approximation Properties"}
    , approximationMethod_{"approximationMethod", "Approximation Method"}
    , importanceVolume_{"importanceVolume", "Optional scalar field with importance data"_help}
    , importanceSumCoefficients_{"importanceSumCoefficients", "Importance sum coefficients"}
    , opticalDepthCoefficients_{"opticalDepthCoefficients", "Optical depth coefficients"}
    , smoothing_{"smoothing", "Smoothing", false}
    , gaussianKernelRadius_{"gaussianKernelRadius", "Gaussian kernel radius", 3, 1, 50}
    , gaussianKernelSigma_{"gaussianKernelSigma", "Gaussian kernel sigma", 1, 0.001, 50} {

    addPort(importanceVolume_).setOptional(true);

    illustrationSettings_.enabled_.setChecked(false);
    removeProperty(illustrationSettings_.enabled_);
    addProperties(q_, r_, lambda_);

    q_.onChange([this]() {
        auto oor = dynamic_cast<OpacityOptimisationRenderer*>(flr_.get());
        if (oor) oor->q = q_;
    });

    r_.onChange([this]() {
        auto oor = dynamic_cast<OpacityOptimisationRenderer*>(flr_.get());
        if (oor) oor->r = r_;
    });

    lambda_.onChange([this]() {
        auto oor = dynamic_cast<OpacityOptimisationRenderer*>(flr_.get());
        if (oor) oor->lambda = lambda_;
    });

    addProperty(opacityOptimisationRenderer_);

    for (auto const& [key, val] : Approximations::approximations) {
        approximationMethod_.addOption(key.c_str(), val.name.c_str(), key);
    }
    approximationMethod_.setDefault("fourier");
    approximationMethod_.set("fourier");

    approximationProperties_.addProperties(approximationMethod_, importanceSumCoefficients_,
                                           opticalDepthCoefficients_, smoothing_);
    smoothing_.addProperties(gaussianKernelRadius_, gaussianKernelSigma_);

    importanceVolume_.onChange([this]() {
        auto oor = dynamic_cast<OpacityOptimisationRenderer*>(flr_.get());
        if (oor) {
            if (importanceVolume_.isReady())
                oor->importanceVolume = &importanceVolume_;
            else {
                oor->importanceVolume = nullptr;
            }
            oor->importanceVolumeDirty = true;
        }
    });

    opacityOptimisationRenderer_.onChange([this]() {
        flr_.reset();
        if (opacityOptimisationRenderer_ == 0) {
            addProperty(approximationProperties_);

            const auto* p = &Approximations::approximations.at(approximationMethod_);
            importanceSumCoefficients_.setMinValue(p->minCoefficients);
            importanceSumCoefficients_.setMaxValue(p->maxCoefficients);
            opticalDepthCoefficients_.setMinValue(p->minCoefficients);
            opticalDepthCoefficients_.setMaxValue(p->maxCoefficients);

            flr_ = std::make_unique<ApproximateOpacityOptimisationRenderer>(
                p, &camera_, importanceSumCoefficients_, opticalDepthCoefficients_,
                gaussianKernelRadius_, gaussianKernelSigma_);
        } else {
            removeProperty(approximationProperties_);
            flr_ = std::make_unique<DecoupledOpacityOptimisationRenderer>(&camera_);
        }
        flrReload_ = flr_->onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

        auto oor = dynamic_cast<OpacityOptimisationRenderer*>(flr_.get());
        if (oor) {
            if (importanceVolume_.isReady())
                oor->importanceVolume = &importanceVolume_;
            else {
                oor->importanceVolume = nullptr;
            }
            oor->importanceVolumeDirty = true;
        }

        q_.propertyModified();
        r_.propertyModified();
        lambda_.propertyModified();
    });

    approximationMethod_.onChange([this]() {
        if (opacityOptimisationRenderer_ == 0) {
            const auto* p = &Approximations::approximations.at(approximationMethod_);
            importanceSumCoefficients_.setMinValue(p->minCoefficients);
            importanceSumCoefficients_.setMaxValue(p->maxCoefficients);
            opticalDepthCoefficients_.setMinValue(p->minCoefficients);
            opticalDepthCoefficients_.setMaxValue(p->maxCoefficients);

            auto aoor = dynamic_cast<ApproximateOpacityOptimisationRenderer*>(flr_.get());
            if (aoor) {
                aoor->setDescriptor(p);
            }
        }
    });

    importanceSumCoefficients_.onChange([this]() {
        if (opacityOptimisationRenderer_ == 0) {
            auto aoor = dynamic_cast<ApproximateOpacityOptimisationRenderer*>(flr_.get());
            if (aoor) aoor->setImportanceSumCoeffs(importanceSumCoefficients_);
        }
    });

    opticalDepthCoefficients_.onChange([this]() {
        if (opacityOptimisationRenderer_ == 0) {
            auto aoor = dynamic_cast<ApproximateOpacityOptimisationRenderer*>(flr_.get());
            if (aoor) aoor->setOpticalDepthCoeffs(opticalDepthCoefficients_);
        }
    });

    smoothing_.onChange([this]() {
        if (opacityOptimisationRenderer_ == 0) {
            auto aoor = dynamic_cast<ApproximateOpacityOptimisationRenderer*>(flr_.get());
            if (aoor) {
                aoor->smoothing = smoothing_;
                if (smoothing_)
                    aoor->generateAndUploadGaussianKernel(gaussianKernelRadius_,
                                                          gaussianKernelSigma_);
            }
        }
    });
    smoothing_.propertyModified();

    gaussianKernelRadius_.onChange([this]() {
        if (opacityOptimisationRenderer_ == 0) {
            auto aoor = dynamic_cast<ApproximateOpacityOptimisationRenderer*>(flr_.get());
            if (aoor)
                aoor->generateAndUploadGaussianKernel(gaussianKernelRadius_, gaussianKernelSigma_);
        }
    });

    gaussianKernelSigma_.onChange([this]() {
        if (opacityOptimisationRenderer_ == 0) {
            auto aoor = dynamic_cast<ApproximateOpacityOptimisationRenderer*>(flr_.get());
            if (aoor)
                aoor->generateAndUploadGaussianKernel(gaussianKernelRadius_, gaussianKernelSigma_);
        }
    });

    opacityOptimisationRenderer_.propertyModified();
}

void OpacityOptimiser::process() { RasterizationRenderer::process(); }

}  // namespace inviwo
