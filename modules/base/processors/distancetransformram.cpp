/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2016 Inviwo Foundation
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

#include "distancetransformram.h"
#include <modules/base/algorithm/dataminmax.h>
#include <modules/base/algorithm/volume/volumeramdistancetransform.h>

namespace inviwo {

const ProcessorInfo DistanceTransformRAM::processorInfo_{
    "org.inviwo.DistanceTransformRAM",  // Class identifier
    "Distance Transform",               // Display name
    "Volume Operation",                 // Category
    CodeState::Experimental,            // Code state
    Tags::CPU,                          // Tags
};
const ProcessorInfo DistanceTransformRAM::getProcessorInfo() const {
    return processorInfo_;
}

DistanceTransformRAM::DistanceTransformRAM()
    : Processor()
    , volumePort_("inputVolume")
    , outport_("outputVolume")
    , transformEnabled_("transformActive", "Enabled", true)
    , threshold_("threshold", "Threshold", 0.5, 0.0, 1.0)
    , flip_("flip", "Flip", false)
    , normalize_("normalize", "Use normalized threshold", true)
    , resultDistScale_("distScale", "Scaling Factor", 1.0f, 0.0f, 1.0e3, 0.05f)
    , resultSquaredDist_("distSquared", "Squared Distance", false)
    , upsample_("upsample", "Up sample", size3_t(1), size3_t(1), size3_t(10))
    , btnForceUpdate_("forceUpdate", "Update Distance Map")
    , volDim_(1u)
    , distTransformDirty_(true) {

    addPort(volumePort_);
    addPort(outport_);

    addProperty(transformEnabled_);
    addProperty(threshold_);
    addProperty(flip_);
    addProperty(normalize_);
    addProperty(resultDistScale_);
    addProperty(resultSquaredDist_);
    addProperty(upsample_);
    addProperty(btnForceUpdate_);

    transformEnabled_.onChange(this, &DistanceTransformRAM::paramChanged);
    btnForceUpdate_.onChange(this, &DistanceTransformRAM::paramChanged);

    progressBar_.hide();
}

DistanceTransformRAM::~DistanceTransformRAM() = default;

void DistanceTransformRAM::process() {
    if (!transformEnabled_.get()) {
        // pass inport to outport
        outport_.setData(volumePort_.getData());
        return;
    }

    if (util::is_future_ready(newVolume_)) {
        outport_.setData(newVolume_.get());
    } else if (!newVolume_.valid()) {  // We are not waiting for a calculation
        if (volumePort_.isChanged() || distTransformDirty_) {
            updateOutport();
        }
    }
}

void DistanceTransformRAM::updateOutport() {
    auto done = [this]() {
        dispatchFront([this]() { invalidate(InvalidationLevel::InvalidOutput); });
    };

    auto calc =
        [
          pb = &progressBar_, upsample = upsample_.get(), threshold = threshold_.get(),
          normalize = normalize_.get(), flip = flip_.get(), square = resultSquaredDist_.get(),
          scale = resultDistScale_.get(), done
        ](std::shared_ptr<const Volume> volume)
            ->std::shared_ptr<const Volume> {

        auto volDim = glm::max(volume->getDimensions(), size3_t(1u));
        auto dstRepr = std::make_shared<VolumeRAMPrecision<float>>(upsample * volDim);

        const auto progress = [pb](double f) {
            dispatchFront([f, pb]() {
                f < 1.0 ? pb->show() : pb->hide();
                pb->updateProgress(f);
            });
        };
        util::volumeDistanceTransform(volume.get(), dstRepr.get(), upsample, threshold, normalize,
                                      flip, square, scale, progress);

        auto dstVol = std::make_shared<Volume>(dstRepr);
        // pass meta data on
        dstVol->setModelMatrix(volume->getModelMatrix());
        dstVol->setWorldMatrix(volume->getWorldMatrix());
        dstVol->copyMetaDataFrom(*volume);

        const dvec3 dim{dstRepr->getDimensions()};
        const auto maxDist = square ? glm::length2(dim) : glm::length(dim);
        dstVol->dataMap_.dataRange = dvec2(0.0, maxDist);
        dstVol->dataMap_.valueRange = dvec2(0.0, maxDist);

        done();
        return dstVol;
    };

    newVolume_ = dispatchPool(calc, volumePort_.getData());

    distTransformDirty_ = false;
}

void DistanceTransformRAM::paramChanged() { distTransformDirty_ = true; }

}  // namespace

