/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <modules/opencl/volume/volumeclconverter.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <modules/opencl/inviwoopencl.h>

namespace inviwo {

std::shared_ptr<VolumeCL> VolumeRAM2CLConverter::createFrom(
    std::shared_ptr<const VolumeRAM> volumeRAM) const {
    size3_t dimensions = volumeRAM->getDimensions();
    const void* data = volumeRAM->getData();
    return std::make_shared<VolumeCL>(dimensions, volumeRAM->getDataFormat(), data);
}

void VolumeRAM2CLConverter::update(std::shared_ptr<const VolumeRAM> volumeSrc,
                                   std::shared_ptr<VolumeCL> volumeDst) const {
    if (volumeSrc->getDimensions() != volumeDst->getDimensions()) {
        volumeDst->setDimensions(volumeSrc->getDimensions());
    }
    volumeDst->upload(volumeSrc->getData());
}

std::shared_ptr<VolumeRAM> VolumeCL2RAMConverter::createFrom(
    std::shared_ptr<const VolumeCL> volumeCL) const {
    size3_t dimensions = volumeCL->getDimensions();
    auto destination = createVolumeRAM(dimensions, volumeCL->getDataFormat());

    if (destination) {
        volumeCL->download(destination->getData());
        // const cl::CommandQueue& queue = OpenCL::getInstance()->getQueue();
        // queue.enqueueReadImage(volumeCL->getVolume(), true, glm::size3_t(0),
        // glm::size3_t(dimension), 0, 0, volumeRAM->getData());
    }

    return destination;
}

void VolumeCL2RAMConverter::update(std::shared_ptr<const VolumeCL> volumeSrc,
                                   std::shared_ptr<VolumeRAM> volumeDst) const {
    if (volumeSrc->getDimensions() != volumeDst->getDimensions()) {
        volumeDst->setDimensions(volumeSrc->getDimensions());
    }

    volumeSrc->download(volumeDst->getData());

    if (volumeDst->hasHistograms()) volumeDst->getHistograms()->setValid(false);
}

}  // namespace
