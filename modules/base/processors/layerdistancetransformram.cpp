/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#include <modules/base/processors/layerdistancetransformram.h>

#include <modules/base/algorithm/dataminmax.h>
#include <modules/base/algorithm/image/layerramdistancetransform.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>

namespace inviwo {

const ProcessorInfo LayerDistanceTransformRAM::processorInfo_{
    "org.inviwo.LayerDistanceTransformRAM",  // Class identifier
    "Image Distance Transform",              // Display name
    "Image Operation",                       // Category
    CodeState::Stable,                       // Code state
    Tags::CPU,                               // Tags
};
const ProcessorInfo LayerDistanceTransformRAM::getProcessorInfo() const {
    return processorInfo_;
}

LayerDistanceTransformRAM::LayerDistanceTransformRAM()
    : Processor()
    , imagePort_("inputImage")
    , outport_("outputImage", DataVec4UInt8::get(), false)
    , threshold_("threshold", "Threshold", 0.5, 0.0, 1.0)
    , flip_("flip", "Flip", false)
    , normalize_("normalize", "Use normalized threshold", true)
    , resultDistScale_("distScale", "Scaling Factor", 1.0f, 0.0f, 1.0e3, 0.05f)
    , resultSquaredDist_("distSquared", "Squared Distance", false)
    , upsampleFactorUniform_("upsampleFactorUniform", "Sampling Factor", 1, 1, 10)
    , upsampleFactorVec2_("upsampleFactorVec2", "Sampling Factor", size2_t(1), size2_t(1),
                          size2_t(10))
    , uniformUpsampling_("uniformUpsampling", "Uniform Upsampling", false)
    , btnForceUpdate_("forceUpdate", "Update Distance Map")
    , distTransformDirty_(true)
    , hasNewData_(false) {

    addPort(imagePort_);
    addPort(outport_);

    addProperty(threshold_);
    addProperty(flip_);
    addProperty(normalize_);
    addProperty(resultDistScale_);
    addProperty(resultSquaredDist_);
    addProperty(uniformUpsampling_);
    addProperty(upsampleFactorVec2_);
    addProperty(upsampleFactorUniform_);

    auto triggerUpdate = [&]() {
        bool uniform = uniformUpsampling_.get();
        upsampleFactorVec2_.setVisible(!uniform);
        upsampleFactorUniform_.setVisible(uniform);
    };
    uniformUpsampling_.onChange(triggerUpdate);
    upsampleFactorUniform_.setVisible(false);

    addProperty(btnForceUpdate_);

    btnForceUpdate_.onChange(this, &LayerDistanceTransformRAM::paramChanged);

    progressBar_.hide();
}

LayerDistanceTransformRAM::~LayerDistanceTransformRAM() = default;

void LayerDistanceTransformRAM::invalidate(InvalidationLevel invalidationLevel, Property* source) {
    notifyObserversInvalidationBegin(this);
    PropertyOwner::invalidate(invalidationLevel, source);
    if (!isValid() && hasNewData_) {
        for (auto& port : getOutports()) port->invalidate(InvalidationLevel::InvalidOutput);
    }
    notifyObserversInvalidationEnd(this);

    if (!isValid() && isSink()) {
        performEvaluateRequest();
    }
}

void LayerDistanceTransformRAM::process() {
    if (util::is_future_ready(newImage_)) {
        try {
            auto image = newImage_.get();
            outport_.setData(image);
            hasNewData_ = false;
            btnForceUpdate_.setDisplayName("Update Distance Map");
        } catch (Exception&) {
            // Need to reset the future, VS bug:
            // http://stackoverflow.com/questions/33899615/stdfuture-still-valid-after-calling-get-which-throws-an-exception
            newImage_ = {};
            outport_.setData(static_cast<Image*>(nullptr));
            hasNewData_ = false;
            throw;
        }
    } else if (!newImage_.valid()) {  // We are not waiting for a calculation
        btnForceUpdate_.setDisplayName("Update Distance Map (dirty)");
        if (imagePort_.isChanged() || distTransformDirty_) {
            updateOutport();
        }
    }
}

void LayerDistanceTransformRAM::updateOutport() {
    auto done = [this]() {
        dispatchFront([this]() {
            distTransformDirty_ = false;
            hasNewData_ = true;
            invalidate(InvalidationLevel::InvalidOutput);
        });
    };

    auto calc =
        [
          pb = &progressBar_,
          upsample = uniformUpsampling_.get() ? size3_t(upsampleFactorUniform_.get())
                                              : upsampleFactorVec2_.get(),
          threshold = threshold_.get(), normalize = normalize_.get(), flip = flip_.get(),
          square = resultSquaredDist_.get(), scale = resultDistScale_.get(), done,
          &cache = imageCache_
        ](std::shared_ptr<const Image> image)
            ->std::shared_ptr<Image> {

        auto imgDim = glm::max(image->getDimensions(), size2_t(1u));

        auto res = cache.getTypedUnused<float>(upsample * imgDim);
        std::shared_ptr<Image> dstImage = res.first;
        LayerRAMPrecision<float>* dstRepr = res.second;

        // pass meta data on
        dstImage->getColorLayer()->setModelMatrix(image->getColorLayer()->getModelMatrix());
        dstImage->getColorLayer()->setWorldMatrix(image->getColorLayer()->getWorldMatrix());
        dstImage->copyMetaDataFrom(*image);

        const auto progress = [pb](double f) {
            dispatchFront([f, pb]() {
                f < 1.0 ? pb->show() : pb->hide();
                pb->updateProgress(f);
            });
        };
        util::layerDistanceTransform(image->getColorLayer(), dstRepr, upsample, threshold,
                                     normalize, flip, square, scale, progress);

        done();
        cache.add(dstImage);
        return dstImage;
    };

    newImage_ = dispatchPool(calc, imagePort_.getData());
}

void LayerDistanceTransformRAM::paramChanged() { distTransformDirty_ = true; }

} // namespace

