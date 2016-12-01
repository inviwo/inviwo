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
#include <thread>

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
    , resultSquaredDist_("distSquared", "Squared Distance", true)
    , resultDistScale_("distScale", "Scaling Factor", 1.0f, 0.0f, 1.0e3, 0.05f)
    , upsample_("upsample", "Up sample", size3_t(1), size3_t(1), size3_t(10))
    , btnForceUpdate_("forceUpdate", "Update Distance Map")
    , volDim_(1u)
    , distTransformDirty_(true)
    , numThreads_(std::thread::hardware_concurrency()) {

    addPort(volumePort_);
    addPort(outport_);

    addProperty(transformEnabled_);
    addProperty(resultSquaredDist_);
    addProperty(resultDistScale_);
    addProperty(upsample_);
    addProperty(btnForceUpdate_);

    transformEnabled_.onChange(this, &DistanceTransformRAM::paramChanged);
    btnForceUpdate_.onChange(this, &DistanceTransformRAM::paramChanged);

    progressBar_.hide();

#ifndef __clang__
    if (numThreads_ == 0) numThreads_ = 2 * omp_get_max_threads();
    LogInfo("max available threads (OpenMP): " << omp_get_max_threads());

    omp_set_num_threads(numThreads_);
    int id;
#pragma omp parallel
    {
        id = omp_get_thread_num();
        if (id == 0) {
            LogInfo("Threads used: " << omp_get_num_threads());
        }
    }
#endif
}

DistanceTransformRAM::~DistanceTransformRAM() = default;

void DistanceTransformRAM::process() {
    if (!transformEnabled_.get()) {
        // pass inport to outport
        outport_.setData(volumePort_.getData());
        return;
    }

    if (!dstRepr_ || volumePort_.isChanged() || upsample_.isModified()) {
        std::shared_ptr<const Volume> srcVolume = volumePort_.getData();
        volDim_ = glm::max(srcVolume->getDimensions(), size3_t(1u));

        if (!dstRepr_ || (dstRepr_->getDimensions() != upsample_.get() * volDim_) ||
            volDist_ == srcVolume) {
            dstRepr_ = std::make_shared<VolumeRAMPrecision<float>>(upsample_.get() * volDim_);
            volDist_ = std::make_shared<Volume>(dstRepr_);
            volDist_->setModelMatrix(srcVolume->getModelMatrix());
            volDist_->setWorldMatrix(srcVolume->getWorldMatrix());
            // pass meta data on
            volDist_->copyMetaDataFrom(*srcVolume);
        }
    }

    if (distTransformDirty_) {
        progressBar_.resetProgress();
        progressBar_.show();

        updateOutport();
        outport_.setData(volDist_);

        progressBar_.finishProgress();
        progressBar_.hide();
    }
}

void DistanceTransformRAM::updateOutport() {
    
    const auto inputVolumeRep = volumePort_.getData()->getRepresentation<VolumeRAM>();
    inputVolumeRep->dispatch<void, dispatching::filter::Scalars>([this](const auto vrprecision) {
        using VolumeType = util::PrecsionType<decltype(vrprecision)>;
        using ValueType = util::PrecsionValueType<decltype(vrprecision)>;
        computeDistanceTransform(vrprecision, dstRepr_.get(), upsample_.get());
        volDist_->invalidateAllOther(dstRepr_.get());
    });
    distTransformDirty_ = false;

    const dvec3 dim{dstRepr_->getDimensions()};
    const auto maxDist = resultSquaredDist_.get() ? glm::length2(dim) : glm::length(dim);
    volDist_->dataMap_.dataRange = dvec2(0.0, maxDist);
    volDist_->dataMap_.valueRange = dvec2(0.0, maxDist);
}

void DistanceTransformRAM::paramChanged() { distTransformDirty_ = true; }

}  // namespace

